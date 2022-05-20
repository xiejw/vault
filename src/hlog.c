#include "hlog.h"

#include "stdio.h"  // delete

static error_t attachNodeToRoot(struct ft_node *root, struct ft_node *node);
static sds_t sdsNewRaw(char *buf, size_t size);
static error_t lookUpDir(struct ft_node *node, sds_t path,
                         _out_ struct ft_node **out);

error_t
hlogToFt(sds_t root_dir, vec_t(struct hlog *) hlogs, struct ft_node **root)
{
        size_t size = vecSize(hlogs);
        if (size == 0) return errNew("empty hlogs are not allowed.");

        *root = ftRootNew(root_dir);

        struct hlog *hl;
        struct ft_node *n;
        for (size_t i = 0; i < size; i++) {
                hl = hlogs[i];
                if (hl->cmd == HLOG_DEL) {
                        return errNew(
                            "HLOG_DEL is not supported yet in hlogToFt. Item "
                            "index: %zu",
                            i);
                }

                sds_t checksum = sdsNew((char *)hl->checksum);
                sds_t path     = sdsNew(hl->path);

                n           = ftNodeNew();
                n->path     = path;
                n->checksum = checksum;
                // parent field will be added by attachNodeToRoot.

                if (attachNodeToRoot(*root, n) != OK)
                        return errEmitNote("failed to handle item index: %zu",
                                           i);
        }

        return OK;
}

error_t
attachNodeToRoot(struct ft_node *root, struct ft_node *node)
{
        // The algorithm is we will scan the node->path and do segment. And
        // then find the dir in root one by one.  We could use a dict to cache
        // result later for performance reason.

        sds_t path = node->path;  // alias

        size_t end     = sdsLen(path);
        size_t current = 0;
        size_t last    = 0;

        sds_t s;

        struct ft_node *old_parent = root;

        while (1) {
                // find the last segement from last to current.
                if (current == end) {
                        if (current == last)
                                return errNew("path segment is empty: %s",
                                              path);
                        // s = sdsNewRaw(path, current );
                        // DEBUG printf("find segment: %s\n", s);
                        node->parent = old_parent;
                        vecPushBack(&old_parent->children, node);
                        return OK;
                }

                if (path[current] == '/') {
                        if (current == last)
                                return errNew("path segment is empty: %s",
                                              path);
                        s = sdsNewRaw(path, current);
                        // DEBUG printf("find segment: %s\n", s);

                        struct ft_node *parent;
                        if (lookUpDir(old_parent, s, &parent)) {
                                return errEmitNote(
                                    "failed to find the parent dir for node.");
                        }
                        old_parent = parent;

                        last = current + 1;
                }
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

        vec_t(struct ft_node *) children = node->children;
        size_t child_count               = vecSize(children);

        struct ft_node *n;
        for (size_t i = 0; i < child_count; i++) {
                n = children[i];
                if (strcmp(n->path, path) == 0) {
                        if (n->is_dir) {
                                *out = n;
                                return OK;
                        }
                        return errNew("path is a file, but expect a dir: %s",
                                      path);
                }
        }

        // new one should be created at this point

        n    = ftNodeNew();
        *out = n;
        vecPushBack(&node->children, n);

        n->parent   = node;
        n->is_dir   = 1;
        n->path     = sdsNew(path);
        n->root_dir = node->root_dir;
        return OK;
}
