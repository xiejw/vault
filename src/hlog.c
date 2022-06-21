#include "hlog.h"

// -----------------------------------------------------------------------------
// private helper prototypes
// -----------------------------------------------------------------------------
static error_t attachNodeToRoot(struct ft_node *root, struct ft_node *node);
static sds_t sdsNewRaw(char *buf, size_t size);
static error_t lookUpDir(struct ft_node *node, _moved_in_ sds_t path,
                         _out_ struct ft_node **out);

// -----------------------------------------------------------------------------
// impl for public apis
// -----------------------------------------------------------------------------
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

// move to pritvate

static error_t
consumeLine(const sds_t s, size_t *index, struct hlog **hlog)
{
        return errNew("hlogFromSds only supports empty sds buffer.");
}

error_t
hlogFromSds(const sds_t s, vec_t(struct hlog *) * hlogs)
{
        size_t size = sdsLen(s);
        size_t i    = 0;
        error_t err = OK;

        // fast path, unlikely.
        if (i == size) return OK;

        struct hlog *hlog;
        size_t line_num = 0;

        while (1) {
                err = consumeLine(s, &i, &hlog);
                if (err != OK) {
                        err = errEmitNote("failed to parse line number: %zu",
                                          line_num);
                        goto exit;
                }
                line_num++;
                if (hlog == NULL) break;  // end of sds.
        }

exit:
        return err;
}

// -----------------------------------------------------------------------------
// impl for private helpers
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
