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

#endif  // FS_WALK_H_
