#ifndef FS_WALK_H_
#define FS_WALK_H_

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
        int is_dir;  // to distingush between emtpy dir and file.
};

struct ft_node *ftRootNew(sds_t root_dir);  // take ownership of root_dir
void ftFree(struct ft_node *root);
void ftDump(int fd, struct ft_node *root);

//------------------------------------------------------------------------------
// walk data sttructure
//------------------------------------------------------------------------------
#define FT_ERROR_OUT 0
#define FT_WARNING   1
#define FT_SILENT    2

struct ft_walk_config {
        int dangling_sym_link;  // see FT_XXX above
};

error_t walkTree(struct ft_node *, const struct ft_walk_config *);

#endif  // FS_WALK_H_
