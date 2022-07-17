#ifndef FS_H_
#define FS_H_

// eva
#include <adt/sds.h>
#include <adt/vec.h>

// -----------------------------------------------------------------------------
// public file tree data structures for ft_node (ft.c)
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
// low level primitives for ft_node (ft.c)
// -----------------------------------------------------------------------------

extern struct ft_node *ftNodeNew(void);           // new node with all zeros
extern void ftNodeFreeShallow(struct ft_node *);  // shallow free
extern void ftSubTreeFree(struct ft_node *node);  // free a subtree
                                                  //
//------------------------------------------------------------------------------
// walk data structure fot ft_node (walk.c)
//------------------------------------------------------------------------------
#define FTW_ERROR_OUT 0
#define FTW_WARNING   1
#define FTW_SILENT    2

struct ft_walk_config {
        int dangling_sym_link;  // see FT_XXX above
};

// ftWalk will take the 'root' agument, which is pre-filled with a legitimate
// tree node with 'root_dir' (empty chilren), walk into that tree, and expands
// the file tree by attaching all nodes to the 'root' node.
//
// Re: fields in each node.
//
// - All node fields are filled except checksum (set with NULL).
//
// Re: symbolic link
//
// - ftWalk will dereference all symbolic links it sees (either dir or file).
//   Loop should be avoided by users. Currently this is not protection or
//   detection for loops.
//
// - For dangling (dead) symbolic links, 'cfg' controls the behavior (error
//   out, flush a warning message, or just keep silent). They will never be
//   linked into the tree at 'root' though.
extern error_t ftWalk(struct ft_node *root, const struct ft_walk_config *cfg);

// -----------------------------------------------------------------------------
// visitor pattern for ft_node (ft_visit.c)
// -----------------------------------------------------------------------------

// The ftVisit visit fn signature users should provide an implementation.
//
// user_data:
//     blindly passed to visit fn from the user_data provided in ftVisit
//     controller (see API next one).
//
// inout flag:
//     (in):
//         0 preorder visit
//         1 postorder visit
//     (out):
//         OR based FTV_xxx output signals below
//
#define FTV_NO_CHANGE 0  // no change to the tree
#define FTV_DETACH \
        1  // ctl detaches the node (other nodes order undefined).
           // if on root note, the whole tree will be freed.
           // if with FTV_PREORDER, the subtree visit will be
           // skipped.
typedef error_t (*ft_visit_fn_t)(void *user_data, struct ft_node *,
                                 _inout_ int *flag);

// The ftVisit controller API
//
// arg:
//     fn: visit func.
//     user_data: passed to visit func for each invocation.
//     root: dir node (must be root)
//     intflag: OR based FTV_xxx input signals above
//
#define FTV_PREORDER  1  // parent node first
#define FTV_POSTORDER 2  // parent node final
#define FTV_BOTHORDER \
        (FTV_PREORDER | FTV_POSTORDER)  // visit dir node twice (if first
                                        // visit is not detach). File node will
                                        // be visted once (preorder).
#define FTV_DIRONLY  4  // dir only  (cannot set with FTV_FILEONLY)
#define FTV_FILEONLY 8  // file only (cannot set with FTV_DIRONLY)

extern error_t ftVisit(ft_visit_fn_t fn, void *user_data, struct ft_node *root,
                       int inflag);

// -----------------------------------------------------------------------------
// supporting visit fns for ft_node.
// -----------------------------------------------------------------------------
extern void ftSort(struct ft_node *root);
extern void ftDump(int fd, struct ft_node *root);
extern void ftDumpSds(_mut_ sds_t *s, struct ft_node *root);
extern void ftTrimEmptyDir(struct ft_node *root);

#endif  // FS_H_
