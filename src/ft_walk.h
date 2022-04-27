#ifndef FS_WALK_H_
#define FS_WALK_H_

// vault
#include "ft.h"

//------------------------------------------------------------------------------
// walk data structure
//------------------------------------------------------------------------------
#define FTW_ERROR_OUT 0
#define FTW_WARNING   1
#define FTW_SILENT    2

struct ft_walk_config {
        int dangling_sym_link;  // see FT_XXX above
};

extern error_t ftWalk(struct ft_node *root, const struct ft_walk_config *cfg);

#endif  // FS_WALK_H_
