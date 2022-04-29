#include "ft_diff.h"

error_t
ftDiff(const struct ft_node *l, const struct ft_node *r,
       _mut_ vec_t(struct ft_node *) * only_in_l,
       _mut_ vec_t(struct ft_node *) * only_in_r)
{
        assert(l != NULL && l->checksum != NULL);
        assert(r != NULL && r->checksum != NULL);

        return OK;
}
