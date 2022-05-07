#include "ft_diff.h"

static error_t ftDiffHandleItem(const struct ft_node *l,
                                const struct ft_node *r,
                                _mut_ vec_t(struct ft_node *) * only_f_in_l,
                                _mut_ vec_t(struct ft_node *) * only_f_in_r);

static error_t ftDiffHandleSubTree(const struct ft_node *l,
                                   const struct ft_node *r,
                                   _mut_ vec_t(struct ft_node *) * only_f_in_l,
                                   _mut_ vec_t(struct ft_node *) * only_f_in_r);

error_t
ftDiff(const struct ft_node *l, const struct ft_node *r,
       _mut_ vec_t(struct ft_node *) * only_f_in_l,
       _mut_ vec_t(struct ft_node *) * only_f_in_r)
{
        assert(l != NULL && l->is_dir && l->checksum != NULL);
        assert(r != NULL && r->is_dir && r->checksum != NULL);

        if (sdsCmp(l->checksum, l->checksum) == 0) {
                return OK;
        }

        return ftDiffHandleSubTree(l, r, only_f_in_l, only_f_in_r);
}

// l and r must be different (types could be different)
error_t
ftDiffHandleItem(const struct ft_node *l, const struct ft_node *r,
                 _mut_ vec_t(struct ft_node *) * only_f_in_l,
                 _mut_ vec_t(struct ft_node *) * only_f_in_r)
{
        if (l->is_dir && r->is_dir) {
                // TODO: walk into and compare again
        } else if (l->is_dir) {
                // TODO
                // PUT all files in l in only_f_in_l
                // PUT file as r in only_f_in_r
        } else if (r->is_dir) {
                // TODO
                // PUT file as l in only_f_in_l
                // PUT all files in r in only_f_in_r
        } else {
                // TODO
                // PUT file as l in only_f_in_l
                // PUT file as r in only_f_in_r
        }

        return errNew("full fn of ftDiff is unimpl");
}

// l and r must be different (both both are tree
static error_t
ftDiffHandleSubTree(const struct ft_node *l, const struct ft_node *r,
                    _mut_ vec_t(struct ft_node *) * only_f_in_l,
                    _mut_ vec_t(struct ft_node *) * only_f_in_r)
{
        assert(l->is_dir && r->is_dir);

        size_t sl = vecSize(l->children);
        size_t sr = vecSize(r->children);
        // fast path
        if (sl == 0 || sr == 0) {
                if (sl == 0) {
                        // PUT all files in r in only_f_in_r
                } else if (sr == 0) {
                        // PUT all files in l in only_f_in_l
                }

                return OK;
        }

        vec_t(struct ft_node *) l_children = l->children;
        vec_t(struct ft_node *) r_children = r->children;
        size_t offset_r = 0;  // items before offset_r (index) in r is handled
        for (size_t i = 0; i < sl; i++) {
                struct ft_node *l_child = l_children[i];

                // TODO bisec (by types) child name in r (j == -1) means not
                // find note the order is type -> name. Is this doable? Only
                // search in type?
                size_t j = -1;

                if (j == -1) {
                        // TODO put file/dir of l_child in only_f_in_l
                        continue;
                }

                if (offset_r < j) {
                        // TODO put file/dir of r_child (from offset_r (include)
                        // to j (exclusive) in only_f_in_r
                }

                offset_r = j;

                struct ft_node *r_child =
                    r_children[j] if (sdsCmp(l_child->checksum ==
                                             r_child->checksum) == 0)
                {
                        assert(l_child->is_dir == r_child->is_dir);
                        continue;
                }

                ftDiffHandleItem(l_child, r_child, only_f_in_l, only_f_in_r);
        }

        for (size_t j = offset_r; j < sr; j++) {
                // TODO put file/dir of r_child (from offset_r (include) to j
                // (exclusive) in only_f_in_r
        }

        return OK;
}
