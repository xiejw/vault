#include "ft.h"

#include <stdio.h>  // dprintf

// -----------------------------------------------------------------------------
// public visit apis impl
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

// -----------------------------------------------------------------------------
// helper impl for visit
// -----------------------------------------------------------------------------

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
                        goto next_iter; /* without advancing i */    \
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

#undef HANDLE_NODE

                // advance to next one
                i++;

        next_iter:;
        }

exit:

        return err;
}

// -----------------------------------------------------------------------------
// public supporting fns impl
// -----------------------------------------------------------------------------

static error_t fdDumpVisitFn(void *data, struct ft_node *node, int *flag);
static error_t ftSortFn(void *data, struct ft_node *node, int *flag);
static error_t ftTrimEmptyDirFn(void *data, struct ft_node *node, int *flag);

struct visit_dump_ctx {
        sds_t space;
        int fd;
        sds_t *buf;  // if NULL write to fd above
};

// dump the tree representation to s (say stdout).
void
ftDumpSds(_mut_ sds_t *s, struct ft_node *root)
{
        struct visit_dump_ctx ctx = {
            .fd = -1, .space = sdsNew("    "), .buf = s};
        sdsCatPrintf(s, "root - %s\n", root->root_dir);
        ftVisit(fdDumpVisitFn, &ctx, root, FTV_BOTHORDER);  // ignore err
        sdsFree(ctx.space);
}

// dump the tree representation to fd (say stdout).
void
ftDump(int fd, struct ft_node *root)
{
        struct visit_dump_ctx ctx = {
            .fd = fd, .space = sdsNew("    "), .buf = NULL};
        dprintf(fd, "root - %s\n", root->root_dir);
        ftVisit(fdDumpVisitFn, &ctx, root, FTV_BOTHORDER);  // ignore err
        sdsFree(ctx.space);
}

void
ftSort(struct ft_node *root)
{
        ftVisit(ftSortFn, NULL, root,
                FTV_PREORDER | FTV_DIRONLY);  // ignore err
}

void
ftTrimEmptyDir(struct ft_node *root)
{
        ftVisit(ftTrimEmptyDirFn, NULL, root,
                FTV_PREORDER | FTV_DIRONLY);  // ignore err
}

// -----------------------------------------------------------------------------
// helper impl for supporting fns
// -----------------------------------------------------------------------------

error_t
fdDumpVisitFn(void *data, struct ft_node *node, int *flag)
{
        int is_dir   = node->is_dir;
        int preorder = (*flag) == 0;

        if (node->parent == NULL) goto exit;

        struct visit_dump_ctx *ctx = (struct visit_dump_ctx *)data;

        if (preorder) {
                if (ctx->buf == NULL)
                        dprintf(ctx->fd, "%s+-> %s%s\n", ctx->space, node->path,
                                is_dir ? " (+)" : "");
                else
                        sdsCatPrintf(ctx->buf, "%s+-> %s%s\n", ctx->space,
                                     node->path, is_dir ? " (+)" : "");
        }

        if (is_dir && preorder) {
                // append 4 spaces.
                sdsCatPrintf(&ctx->space, "    ");
        }

        if (is_dir && !preorder) {
                // remove 4 spaces
                size_t len = sdsLen(ctx->space);
                sdsSetLen(ctx->space, len - 4);
                ctx->space[len - 4] = 0;
        }

exit:

        *flag = FTV_NO_CHANGE;
        return OK;
}

error_t
ftSortFn(void *data, struct ft_node *node, int *flag)
{
        (void)data;
        assert(*flag == 0);
        assert(node->is_dir);

        // stupid bubble sort here
        size_t ct = vecSize(node->children);
        if (ct == 0) goto exit;

        for (size_t i = 0; i < ct; i++) {
                for (size_t j = i + 1; j < ct; j++) {
                        struct ft_node *l = node->children[i];
                        struct ft_node *r = node->children[j];

                        int score = 0;
                        int cmp   = strcmp(l->path, r->path);
                        if (cmp > 0) score++;
                        if (cmp < 0) score--;
                        if (l->is_dir) score -= 10;
                        if (r->is_dir) score += 10;

                        if (score > 0) {
                                node->children[j] = l;
                                node->children[i] = r;
                        }
                }
        }

exit:
        *flag = FTV_NO_CHANGE;
        return OK;
}

error_t
ftTrimEmptyDirFn(void *data, struct ft_node *node, int *flag)
{
        assert(node->is_dir);
        assert(*flag == 0);

        if (vecSize(node->children) == 0) {
                *flag = FTV_DETACH;
        } else {
                *flag = FTV_NO_CHANGE;
        }
        return OK;
}
