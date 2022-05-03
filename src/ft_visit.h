#ifndef FS_VISIT_H
#define FS_VISIT_H

#include "ft.h"

// -----------------------------------------------------------------------------
// visitor pattern
// -----------------------------------------------------------------------------

// output signal to ftVisit controler (ctl)
#define FTV_NO_CHANGE 0  // no change to the tree
#define FTV_DETACH \
        1  // ctl detaches the node (other nodes order undefined).
           // if on root note, the whole tree will be freed.
           // if with FTV_PREORDER, the subtree visit will be
           // skipped.

// inout:
//     outflag (in):
//         0 preorder visit
//         1 postorder visit
//     outflag (out):
//         OR based FTV_xxx output signals above
typedef error_t (*ft_visit_fn_t)(void *data, struct ft_node *,
                                 _out_ int *outflag);

// input signal to ftVisit controler (ctl)
#define FTV_PREORDER  1  // parent node first
#define FTV_POSTORDER 2  // parent node final
#define FTV_BOTHORDER \
        (FTV_PREORDER | FTV_POSTORDER)  // visit dir node twice (if first
                                        // visit is not detach)

// in:
//     intflag: OR based FTV_xxx input signals above
extern error_t ftVisit(ft_visit_fn_t fn, void *data, struct ft_node *root,
                       int inflag);

// -----------------------------------------------------------------------------
// supporting fns
// -----------------------------------------------------------------------------
extern void ftDump(int fd, struct ft_node *root);

#endif  // FS_VISIT_H
