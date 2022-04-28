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
