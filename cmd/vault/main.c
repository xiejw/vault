#include <stdio.h>

// eva
#include <adt/sds.h>
#include <base/log.h>
#include <base/types.h>

// vault
#include "ft_walk.h"

int
main()
{
        error_t err;

        struct ft_node *root      = ftRootNew(sdsNew("tests/a"));
        struct ft_walk_config cfg = {
            .dangling_sym_link = FT_WARNING,
        };

        err = walkTree(root, &cfg);
        if (err) {
                logFatal("fatal error");
                errDump("failed to list files.");
        }

        ftDump(1, root);

        ftFree(root);
        return 0;
}
