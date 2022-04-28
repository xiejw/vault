#ifndef FS_VISIT_H
#define FS_VISIT_H

#include "ft.h"

// -----------------------------------------------------------------------------
// visitor pattern
// -----------------------------------------------------------------------------

// output signal to ftVisit controler (ctl)
#define FTV_NO_CHANGE 0  // no change to the tree
#define FTV_DETACH    1  // ctl detaches the node (other nodes order undefined)

// out:
//     outflag: OR based FTV_xxx output signals above
typedef error_t (*ft_visit_fn_t)(void *data, struct ft_node *,
                                 _out_ int *outflag);

// input signal to ftVisit controler (ctl)
#define FTV_PREORDER  0  // parent node first
#define FTV_POSTORDER 1  // parent node final

// in:
//     intflag: OR based FTV_xxx input signals above
error_t ftVisit(ft_visit_fn_t fn, void *data, struct ft_node *root, int inflag);

#endif  // FS_VISIT_H
