#ifndef FS_DIFF_H
#define FS_DIFF_H

// eva
#include <adt/vec.h>

// vault
#include "ft.h"

// -----------------------------------------------------------------------------
// public apis
// -----------------------------------------------------------------------------
extern error_t ftDiff(const struct ft_node *l, const struct ft_node *r,
                      _mut_ vec_t(struct ft_node *) only_in_l,
                      _mut_ vec_t(struct ft_node *) only_in_r);

#endif  // FS_DIFF_H
