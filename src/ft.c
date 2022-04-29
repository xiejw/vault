#include "ft.h"

// -----------------------------------------------------------------------------
// helper prototypes
// -----------------------------------------------------------------------------

// free the subtree at 'node', where 'node' can be a leaf.
static void ftSubTreeFree(struct ft_node *node);

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
        if (p->checksum != NULL) sdsFree(p->checksum);
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
