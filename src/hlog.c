#include "hlog.h"

#include <stdio.h>

// -----------------------------------------------------------------------------
// section: hlogToFt
// -----------------------------------------------------------------------------

static error_t attachNodeToRoot(struct ft_node *root, struct ft_node *node);
static sds_t sdsNewRaw(char *buf, size_t size);
static error_t lookUpDir(struct ft_node *node, _moved_in_ sds_t path,
                         _out_ struct ft_node **out);

error_t
hlogToFt(sds_t root_dir, vec_t(struct hlog *) hlogs, struct ft_node **root)
{
        size_t hlog_count = vecSize(hlogs);
        if (hlog_count == 0) return errNew("empty hlogs are not allowed.");

        const struct hlog *hl;
        struct ft_node *n;
        *root = ftRootNew(root_dir);

        for (size_t i = 0; i < hlog_count; i++) {
                hl = hlogs[i];
                if (hl->cmd == HLOG_DEL) {
                        return errNew(
                            "HLOG_DEL is not supported yet in hlogToFt. Item "
                            "index: %zu",
                            i);
                }

                assert(hl->cmd == HLOG_ADD);

                // both sds are owned by node 'n'.
                sds_t checksum = sdsNew((const char *)hl->checksum);
                sds_t path     = sdsNew(hl->path);

                n           = ftNodeNew();
                n->path     = path;      // moved in
                n->checksum = checksum;  // moved in
                // Note: 'parent' field will be added by attachNodeToRoot.

                if (attachNodeToRoot(*root, n) != OK)
                        return errEmitNote(
                            "failed to handle item index (%zu) for path: %s", i,
                            path);
        }

        return OK;
}

// -----------------------------------------------------------------------------
// impl for private helpers for hlogToFt
// -----------------------------------------------------------------------------

error_t
attachNodeToRoot(struct ft_node *root, struct ft_node *node)
{
        // The algorithm is:
        //
        // We will scan the node->path and look up for parent dir segment by
        // segment. We could use a dict to cache result later for performance
        // reason.

        sds_t path_alias = node->path;  // alias

        size_t end     = sdsLen(path_alias);
        size_t current = 0;
        size_t last    = 0;
        sds_t s;  // tmp var. ownership depends on call site.

        struct ft_node *old_parent = root;
        // loop over segments
        while (1) {
                // found the last segment.
                if (current == end) {
                        if (current == last)
                                return errNew(
                                    "there is an empty path segment (this "
                                    "means the path is invalid): %s",
                                    path_alias);

                        node->parent = old_parent;
                        vecPushBack(&old_parent->children, node);
                        return OK;
                }

                // found a segment (not the last one)
                if (path_alias[current] == '/') {
                        if (current == last)
                                return errNew(
                                    "there is an empty path segment (this "
                                    "means the path is invalid): %s",
                                    path_alias);

                        s = sdsNewRaw(path_alias, current);

                        struct ft_node *parent;
                        if (OK != lookUpDir(old_parent, s, &parent)) {
                                return errEmitNote(
                                    "failed to find the parent dir for node.");
                        }

                        old_parent = parent;
                        last       = current + 1;
                }

                // keep looking for next segment.
                current++;
        }

        return OK;
}

sds_t
sdsNewRaw(char *buf, size_t size)
{
        sds_t s = sdsEmptyWithCap(size);
        memcpy(s, buf, size);
        s[size] = 0;
        sdsSetLen(s, size);
        return s;
}

error_t
lookUpDir(struct ft_node *node, sds_t path, struct ft_node **out)
{
        assert(node != NULL);
        assert(node->is_dir);

        vec_t(struct ft_node *) children = node->children;  // alias
        size_t child_count               = vecSize(children);

        error_t err = OK;
        struct ft_node *n;
        for (size_t i = 0; i < child_count; i++) {
                n = children[i];
                if (strcmp(n->path, path) == 0) {
                        if (n->is_dir) {
                                *out = n;
                                goto exit;
                        }

                        err = errNew("path is a file, but expect a dir: %s",
                                     path);
                        goto exit;
                }
        }

        // new one should be created at this point
        n           = ftNodeNew();
        n->parent   = node;
        n->is_dir   = 1;
        n->path     = path;
        n->root_dir = node->root_dir;

        *out = n;
        vecPushBack(&node->children, n);

        return OK;

exit:
        sdsFree(path);
        return err;
}

// -----------------------------------------------------------------------------
// section: hlogFromSds
// -----------------------------------------------------------------------------

static error_t
consumeOneChar(const sds_t s, _inout_ size_t *index, const size_t size,
               _inout_ char *c, char *err_msg)
{
        if (*index >= size)
                return errNew("expected to see %s, but got EOF", err_msg);

        *c = s[(*index)++];
        return OK;
}

static error_t
consumeOp(const sds_t s, size_t *index, size_t size, struct hlog *hl)
{
        char c;
        if (OK != consumeOneChar(s, index, size, &c, "hlog op (+/-)")) {
                return errNum();
        }

        switch (c) {
        case '+':
                hl->cmd = 1;
                break;
        case '-':
                hl->cmd = 0;
                break;
        default:
                return errNew("expected to see hlog op (+/-), but got %c", c);
        }
        return OK;
}

static error_t
consumeSpace(const sds_t s, size_t *index, size_t size)
{
        char c;
        if (OK != consumeOneChar(s, index, size, &c, "single space")) {
                return errNum();
        }
        if (c != ' ')
                return errNew("expected to see single space, but got %c", c);
        return OK;
}

static error_t
consumeNewLine(const sds_t s, size_t *index, size_t size)
{
        char c;
        if (OK != consumeOneChar(s, index, size, &c, "newline")) {
                return errNum();
        }
        if (c != '\n') return errNew("expected to see newline, but got %c", c);

        return OK;
}

static error_t
consumeTimestamp(const sds_t s, size_t *index, size_t size, struct hlog *hl)
{
        char timestamp_str[10 + 1];
        u64_t timestamp;

        size_t start = *index;
        size_t end   = start + 1;
        char c;

        // validate leading digit
        if (OK !=
            consumeOneChar(s, index, size, &c, "leading digit for timestamp")) {
                return errNum();
        }

        if (c < '1' || c > '9') {
                return errNew("expected to see leading digit [1-9] but got %c",
                              c);
        }

        // obtain the whole timestamp digits
        while (end < size) {
                c = s[end++];
                if (c < '0' || c > '9') {
                        end--;
                        break;
                }
        }

        // check the total length
        size_t len = end - start;
        if (len > 10) {
                return errNew(
                    "expected to see valid epoch secs but got %.*s (too large)",
                    (int)len, s + start);
        }

        memcpy(timestamp_str, s + start, len);
        timestamp_str[len] = '\0';
        timestamp          = (u64_t)strtol(timestamp_str, NULL, 10);

        // check the error case
        if (timestamp == 0) {
                return errNew("invalid string for timestamp: %s",
                              timestamp_str);
        }

        hl->timestamp = timestamp;
        *index        = end;
        return OK;
}

static error_t
consumeChecksum()
{
        return OK;
}
static error_t
consumeFilePath(struct hlog *hl)
{
        hl->path = sdsEmpty();
        return OK;
}

static error_t
consumeLine(const sds_t s, size_t *index, size_t size, struct hlog **hlog)
{
        error_t err = OK;
        size_t i    = *index;

        if (i == size) {  // end of sds
                *hlog = NULL;
                return OK;
        }

        struct hlog *hl = malloc(sizeof(*hl));

#define OK_OR_EXIT(exp)            \
        do {                       \
                err = (exp);       \
                if (OK != err) {   \
                        goto exit; \
                }                  \
        } while (0)

        OK_OR_EXIT(consumeOp(s, index, size, hl));
        OK_OR_EXIT(consumeSpace(s, index, size));
        OK_OR_EXIT(consumeTimestamp(s, index, size, hl));
        OK_OR_EXIT(consumeSpace(s, index, size));
        OK_OR_EXIT(consumeChecksum());
        OK_OR_EXIT(consumeSpace(s, index, size));
        OK_OR_EXIT(consumeFilePath(hl));
        OK_OR_EXIT(consumeNewLine(s, index, size));

#undef OK_OR_EXIT

exit:
        if (err == OK) {
                *hlog = hl;
        } else {
                free(hl);
        }
        return err;
}

error_t
hlogFromSds(const sds_t s, vec_t(struct hlog *) * hlogs)
{
        size_t size = sdsLen(s);
        size_t i    = 0;
        error_t err = OK;

        struct hlog *hlog;
        size_t line_num = 0;

        while (1) {
                err = consumeLine(s, &i, size, &hlog);
                if (err != OK) {
                        err = errEmitNote("failed to parse line number: %zu",
                                          line_num);
                        goto exit;
                }
                line_num++;
                if (hlog == NULL) break;  // end of sds.
                vecPushBack(hlogs, hlog);
        }

exit:
        return err;
}

// -----------------------------------------------------------------------------
// impl for private helpers for hlogFromSds
// -----------------------------------------------------------------------------
