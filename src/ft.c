#include "ft.h"

#include <stdio.h>  // dprintf

// -----------------------------------------------------------------------------
// helper prototypes
// -----------------------------------------------------------------------------

// free the subtree at 'node', where 'node' can be a leaf.
static void ftSubTreeFree(struct ft_node *node);

static void ftDumpImpl(int fd, struct ft_node *node, sds_t space);

// -----------------------------------------------------------------------------
// public apis impl
// -----------------------------------------------------------------------------
struct ft_node *
ftRootNew(sds_t root_dir)
{
        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmptyWithCap(0);  // never grow
        root->is_dir         = 1;

        // TODO (check is a dir)
        return root;
}

// free the entire tree rooted at 'root'
void
ftFree(struct ft_node *root)
{
        assert(root->parent == NULL);
        ftSubTreeFree(root);
}

// dump the tree representation to fd (say stdout).
void
ftDump(int fd, struct ft_node *root)
{
        sds_t space = sdsEmpty();
        dprintf(fd, "%sroot - %s\n", space, root->root_dir);

        ftDumpImpl(fd, root, space);
        sdsFree(space);
}

struct ft_node *
ftNodeNew(void)
{
        return calloc(1, sizeof(struct ft_node));
}

void
ftNodeFreeShallow(struct ft_node *p)
{
        const int is_root = p->parent == NULL;

        if (is_root) {
                sdsFree(p->root_dir);
        }
        sdsFree(p->path);
        vecFree(p->children);  // shallow free
        free(p);
}

// -----------------------------------------------------------------------------
// helper impl
// -----------------------------------------------------------------------------

static void
ftSubTreeFree(struct ft_node *node)
{
        // dfs walk, might overflow stacks.
        vec_t(struct ft_node *) children = node->children;
        const size_t child_count         = vecSize(children);

        for (size_t i = 0; i < child_count; i++) {
                ftSubTreeFree(children[i]);
        }

        ftNodeFreeShallow(node);
}

static void
ftDumpImpl(int fd, struct ft_node *node, sds_t space)
{
        int is_dir;
        // append 4 spaces.
        sdsCatPrintf(&space, "    ");

        for (size_t i = 0; i < vecSize(node->children); i++) {
                struct ft_node *child = node->children[i];
                is_dir                = child->is_dir;
                dprintf(fd, "%s+-> %s%s\n", space, child->path,
                        is_dir ? " (+)" : "");

                if (is_dir) {
                        ftDumpImpl(fd, child, space);
                }
        }

        // remove 4 spaces
        size_t len = sdsLen(space);
        sdsSetLen(space, len - 4);
        space[len - 4] = 0;
}
