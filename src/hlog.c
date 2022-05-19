#include "hlog.h"

struct ft_node *
hlogToFt(sds_t root_dir, vec_t(struct hlog *) hlogs)
{
        size_t size = vecSize(hlogs);
        if (size == 0) return NULL;

        struct ft_node *root = ftRootNew(root_dir);
        return root;
}
