#ifndef FS_H_
#define FS_H_

// eva
#include <adt/sds.h>
#include <adt/vec.h>

//------------------------------------------------------------------------------
// public file tree data structures.
//------------------------------------------------------------------------------

struct ft_node {
        struct ft_node *parent;  // unowned. if NULL, this is root.
        sds_t root_dir;          // owned by root; alias for non-root.
        sds_t path;              // owned. exclude the root_dir. "" for root.
        vec_t(struct ft_node *) children;  // owned. must be NULL for file.
        int is_dir;  // to distinguish between empty dir and file.
};

extern struct ft_node *ftRootNew(sds_t root_dir);  // take ownership of root_dir
extern void ftFree(struct ft_node *root);

// low level primitives
extern void ftDump(int fd, struct ft_node *root);

// create a new node with zeros filled in all fields.
extern struct ft_node *ftNodeNew(void);
// free a single node (will not free the children it owns).
extern void ftNodeFree(struct ft_node *);

#endif  // FS_H_
