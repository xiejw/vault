#include <stdio.h>

// eva
#include <adt/sds.h>
#include <base/log.h>
#include <base/types.h>

// vault
#include <ft_visit.h>
#include <ft_walk.h>

int
main(int argc, const char **argv)
{
        error_t err;

        const char *root_dir = "tests/test_tree/a";

        if (argc > 1) root_dir = argv[1];

        logInfo("root dir: %s", root_dir);

        struct ft_node *root = ftRootNew(sdsNew(root_dir));

        struct ft_walk_config cfg = {
            .dangling_sym_link = FTW_WARNING,
        };

        err = ftWalk(root, &cfg);
        if (err) {
                logFatal("fatal error");
                errDump("failed to walk the tree at %s", root->root_dir);
        }

        ftDump(1, root);

        ftFree(root);
        return 0;
}
