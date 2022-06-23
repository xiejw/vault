#ifndef FS_VISIT_H
#define FS_VISIT_H

#include "ft.h"

// -----------------------------------------------------------------------------
// visitor pattern
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
// supporting fns
// -----------------------------------------------------------------------------
extern void ftSort(struct ft_node *root);
extern void ftDump(int fd, struct ft_node *root);
extern void ftDumpSds(_mut_ sds_t *s, struct ft_node *root);
extern void ftTrimEmptyDir(struct ft_node *root);

#endif  // FS_VISIT_H
