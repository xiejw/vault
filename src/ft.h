#ifndef FS_H_
#define FS_H_

// eva
#include <adt/sds.h>
#include <adt/vec.h>

// -----------------------------------------------------------------------------
// public file tree data structures
// -----------------------------------------------------------------------------

struct ft_node {
        struct ft_node *parent;  // unowned. if NULL, this is root.
        sds_t root_dir;          // owned by root; alias for non-root.
        sds_t path;              // owned. exclude the root_dir. "" for root.
        vec_t(struct ft_node *) children;  // owned.
        int is_dir;                        // value range: {0, 1}
};

extern struct ft_node *ftRootNew(_moved_in_ sds_t root_dir);
extern void ftFree(struct ft_node *root);  // free the entire tree

// -----------------------------------------------------------------------------
// low level primitives
// -----------------------------------------------------------------------------

extern struct ft_node *ftNodeNew(void);           // new node with all zeros
extern void ftNodeFreeShallow(struct ft_node *);  // shallow free

#endif  // FS_H_
