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
extern void ftDump(int fd, struct ft_node *root);

// create a new node with zeros filled in all fields
extern struct ft_node *ftNodeNew(void);
// free a single node (will not free the child nodes it owns)
extern void ftNodeFreeShallow(struct ft_node *);

// -----------------------------------------------------------------------------
// visitor pattern
// -----------------------------------------------------------------------------

#define FTV_NO_CHANGE 0  // no change to the tree
#define FTV_DETACH    1  // ftVisit will detach the current node (order undefined)

// outflag is the OR based FTV_xxx values
typedef error_t (*ft_visit_fn_t)(void *data, struct ft_node *,
                                 _out_ int *outflag);

// arg:
//   order: 0 pre-order 1 post order
error_t ftVisit(ft_visit_fn_t fn, void *data, struct ft_node *root, int order);

#endif  // FS_H_
