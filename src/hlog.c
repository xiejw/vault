#include "hlog.h"

error_t attachNodeToRoot(struct ft_node *root, struct ft_node *node);

error_t
hlogToFt(sds_t root_dir, vec_t(struct hlog *) hlogs, struct ft_node **root)
{
        size_t size = vecSize(hlogs);
        if (size == 0) return errNew("empty hlogs are not allowed.");

        *root = ftRootNew(root_dir);

        struct hlog *hl;
        struct ft_node *n;
        for (size_t i = 0; i < size; i++) {
                hl = hlogs[i];
                if (hl->cmd == HLOG_DEL) {
                        return errNew(
                            "HLOG_DEL is not supported yet in hlogToFt. Item "
                            "index: %zu",
                            i);
                }

                sds_t checksum = sdsNew((char *)hl->checksum);
                sds_t path     = sdsNew(hl->path);

                n           = ftNodeNew();
                n->path     = path;
                n->checksum = checksum;

                if (attachNodeToRoot(*root, n) != OK)
                        return errEmitNote("failed to handle item index: %zu",
                                           i);
        }

        return OK;
}

error_t
attachNodeToRoot(struct ft_node *root, struct ft_node *node)
{
        // TODO create path to root
        return OK;
}
