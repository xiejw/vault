#ifndef FS_DIFF_H
#define FS_DIFF_H

// eva
#include <adt/vec.h>

// vault
#include "ft.h"

// -----------------------------------------------------------------------------
// public apis for diff
// -----------------------------------------------------------------------------

// generate the diff between tree 'l' and tree 'r'.
//
// Assumption: No both trees have no empty dirs and are sorted.
//
// Semantics: Starting from tree 'l', we will delete file items in 'only_in_l'
// from 'l', and add file items in 'only_in_r' (created parent dirs if needed)
// and them trim all empty dirs. Then the result tree is identical to 'r'.
extern error_t ftDiff(const struct ft_node *l, const struct ft_node *r,
                      _mut_ vec_t(struct ft_node *) * only_f_in_l,
                      _mut_ vec_t(struct ft_node *) * only_f_in_r);

#endif  // FS_DIFF_H
