#include "ft_visit.h"

#include <stdio.h>  // dprintf

// -----------------------------------------------------------------------------
// helper prototypes
// -----------------------------------------------------------------------------

static void ftDumpImpl(int fd, struct ft_node *node, _mut_ sds_t *space);

// -----------------------------------------------------------------------------
// public apis impl
// -----------------------------------------------------------------------------

// dump the tree representation to fd (say stdout).
void
ftDump(int fd, struct ft_node *root)
{
        sds_t space = sdsEmpty();
        dprintf(fd, "%sroot - %s\n", space, root->root_dir);

        ftDumpImpl(fd, root, &space);
        sdsFree(space);
}

static error_t ftVisitImpl(ft_visit_fn_t fn, void *data, struct ft_node *node,
                           int preorder);

error_t
ftVisit(ft_visit_fn_t fn, void *data, struct ft_node *root, int inflag)
{
        int preorder = inflag == FTV_PREORDER;

        error_t err = OK;
        int outflag;

#define HANDLE_ROOT()                                                      \
        do {                                                               \
                outflag = FTV_NO_CHANGE;                                   \
                err     = fn(data, root, &outflag);                        \
                if (err) goto exit;                                        \
                                                                           \
                assert(outflag == FTV_DETACH || outflag == FTV_NO_CHANGE); \
                                                                           \
                if (outflag == FTV_DETACH) {                               \
                        ftFree(root);                                      \
                        goto exit;                                         \
                }                                                          \
        } while (0)

        // special case for root node only.
        if (preorder) {
                HANDLE_ROOT();
        }

        err = ftVisitImpl(fn, data, root, preorder);
        if (err) goto exit;

        // special case for root node only.
        if (!preorder) {
                HANDLE_ROOT();
        }

#undef HANDLE_ROOT

exit:
        return err;
}

error_t
ftVisitImpl(ft_visit_fn_t fn, void *data, struct ft_node *node, int preorder)
{
        error_t err = OK;
        int outflag;

        // loop each child and take child action at parent level.
        for (size_t i = 0; i < vecSize(node->children);) {
                struct ft_node *child = node->children[i];

#define HANDLE_NODE(p, c)                                                  \
        do {                                                               \
                outflag = FTV_NO_CHANGE;                                   \
                err     = fn(data, (c), &outflag);                         \
                if (err) goto exit;                                        \
                                                                           \
                assert(outflag == FTV_DETACH || outflag == FTV_NO_CHANGE); \
                                                                           \
                if (outflag == FTV_DETACH) {                               \
                        ftSubTreeFree((c));                                \
                                                                           \
                        /* swap the element with final one. */             \
                        size_t s         = vecSize((p)->children);         \
                        (p)->children[i] = (p)->children[s - 1];           \
                        vecSetSize((p)->children, s - 1);                  \
                        continue; /* without advancing i */                \
                }                                                          \
        } while (0)

                if (preorder) {
                        HANDLE_NODE(node, child);
                }

                err = ftVisitImpl(fn, data, child, preorder);
                if (err) goto exit;

                if (!preorder) {
                        HANDLE_NODE(node, child);
                }

                // advance to next one
                i++;

#undef HANDLE_NODE
        }

exit:

        return err;
}

// -----------------------------------------------------------------------------
// helper impl
// -----------------------------------------------------------------------------

static void
ftDumpImpl(int fd, struct ft_node *node, sds_t *space)
{
        int is_dir;
        // append 4 spaces.
        sdsCatPrintf(space, "    ");

        for (size_t i = 0; i < vecSize(node->children); i++) {
                struct ft_node *child = node->children[i];
                is_dir                = child->is_dir;
                dprintf(fd, "%s+-> %s%s\n", *space, child->path,
                        is_dir ? " (+)" : "");

                if (is_dir) {
                        ftDumpImpl(fd, child, space);
                }
        }

        // remove 4 spaces
        size_t len = sdsLen(*space);
        sdsSetLen(*space, len - 4);
        (*space)[len - 4] = 0;
}
