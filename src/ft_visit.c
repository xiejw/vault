#include "ft_visit.h"

#include <stdio.h>  // dprintf

// -----------------------------------------------------------------------------
// helper prototypes
// -----------------------------------------------------------------------------

static void ftDumpImpl(int fd, struct ft_node *node, _mut_ sds_t *space);

// -----------------------------------------------------------------------------
// public apis impl
// -----------------------------------------------------------------------------

static error_t ftVisitImpl(ft_visit_fn_t fn, void *data, struct ft_node *node,
                           int preorder, int postorder, int dir_ok,
                           int file_ok);

error_t
ftVisit(ft_visit_fn_t fn, void *data, struct ft_node *root, int inflag)
{
        // cannot be set both
        assert(!((inflag & FTV_FILEONLY) && (inflag & FTV_DIRONLY)));

        // root must be dir
        assert(root->is_dir);

        int preorder  = (inflag & FTV_PREORDER);
        int postorder = (inflag & FTV_POSTORDER);

        int dir_ok  = (inflag & FTV_FILEONLY) == 0;
        int file_ok = (inflag & FTV_DIRONLY) == 0;

        error_t err = OK;
        int flag;

#define HANDLE_ROOT(of)                                              \
        do {                                                         \
                flag = (of);                                         \
                err  = fn(data, root, &flag);                        \
                if (err) goto exit;                                  \
                                                                     \
                assert(flag == FTV_DETACH || flag == FTV_NO_CHANGE); \
                                                                     \
                if (flag == FTV_DETACH) {                            \
                        ftFree(root);                                \
                        goto exit;                                   \
                }                                                    \
        } while (0)

        // special case for root node only.
        if (dir_ok && preorder) {
                HANDLE_ROOT(0);
        }

        err = ftVisitImpl(fn, data, root, preorder, postorder, dir_ok, file_ok);
        if (err) goto exit;

        // special case for root node only.
        if (dir_ok && postorder) {
                HANDLE_ROOT(1);
        }

#undef HANDLE_ROOT

exit:
        return err;
}

error_t
ftVisitImpl(ft_visit_fn_t fn, void *data, struct ft_node *node, int preorder,
            int postorder, int dir_ok, int file_ok)
{
        error_t err = OK;
        int flag;

        // loop each child and take child action at parent level.
        for (size_t i = 0; i < vecSize(node->children);) {
                struct ft_node *child = node->children[i];

#define HANDLE_NODE(p, c, of)                                        \
        do {                                                         \
                flag = (of);                                         \
                err  = fn(data, (c), &flag);                         \
                if (err) goto exit;                                  \
                                                                     \
                assert(flag == FTV_DETACH || flag == FTV_NO_CHANGE); \
                                                                     \
                if (flag == FTV_DETACH) {                            \
                        ftSubTreeFree((c));                          \
                                                                     \
                        /* swap the element with final one. */       \
                        size_t s         = vecSize((p)->children);   \
                        (p)->children[i] = (p)->children[s - 1];     \
                        vecSetSize((p)->children, s - 1);            \
                        continue; /* without advancing i */          \
                }                                                    \
        } while (0)

                if (child->is_dir) {  // dir node
                        if (dir_ok && preorder) {
                                HANDLE_NODE(node, child, 0);
                        }

                        // for dir, we will walk into the tree regardless
                        // dir_ok bit.
                        err = ftVisitImpl(fn, data, child, preorder, postorder,
                                          dir_ok, file_ok);
                        if (err) goto exit;

                        if (dir_ok && postorder) {
                                HANDLE_NODE(node, child, 1);
                        }
                } else if (file_ok) {  // file node. visit file node only once
                        if (preorder) {
                                HANDLE_NODE(node, child, 0);
                        } else if (postorder) {
                                HANDLE_NODE(node, child, 1);
                        }
                }

                // advance to next one
                i++;

#undef HANDLE_NODE
        }

exit:

        return err;
}

// -----------------------------------------------------------------------------
// public supporting fns impl
// -----------------------------------------------------------------------------

void
ftDumpSds(_mut_ sds_t *s, struct ft_node *root)
{
}

// dump the tree representation to fd (say stdout).
void
ftDump(int fd, struct ft_node *root)
{
        sds_t space = sdsEmpty();
        dprintf(fd, "%sroot - %s\n", space, root->root_dir);

        ftDumpImpl(fd, root, &space);
        sdsFree(space);
}

// -----------------------------------------------------------------------------
// helper impl
// -----------------------------------------------------------------------------

// static error_t fdDumpVisitFn(void * data, struct ft_node* node, int* outflag)
// {
//         int is_dir = node->is_dir;
//
//         // print node it self and then. We need two hooks ....
//         return OK;
// }

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
