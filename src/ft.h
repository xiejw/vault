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
        sds_t checksum;                    // owned. Nullable
};

// Creates a single node representing the root. Its chilren is not filled.
// Consider to call ftWalk to fill the real file system tree into this root
// node.
//
// Sample code
//
//     struct ft_node *root = ftRootNew(sdsNew(root_dir));
//
//     struct ft_walk_config cfg = {
//         .dangling_sym_link = FTW_WARNING,
//     };
//
//     err = ftWalk(root, &cfg);
//     if (err) {
//         // handle errors.
//     }
//
//     ftDump(1, root); // print the entire tree.
//
//     ftFree(root);
extern struct ft_node *ftRootNew(_moved_in_ sds_t root_dir);

// Frees all resources for the entire tree pointed by the root node.
extern void ftFree(struct ft_node *root);

// -----------------------------------------------------------------------------
// low level primitives
// -----------------------------------------------------------------------------

extern struct ft_node *ftNodeNew(void);           // new node with all zeros
extern void ftNodeFreeShallow(struct ft_node *);  // shallow free
extern void ftSubTreeFree(struct ft_node *node);  // free a subtree

#endif  // FS_H_
