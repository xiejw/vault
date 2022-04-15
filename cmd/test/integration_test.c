#include "testing/testing.h"

// eva
#include <adt/sds.h>
#include <base/types.h>

// vault
#include "ft_walk.h"

static char *
test_walk_tree()
{
        error_t err;
        struct ft_node *root = ftRootNew(sdsNew("tests/test_tree/a"));

        struct ft_walk_config cfg = {
            .dangling_sym_link = FT_SILENT,  // for testing, we ignore it.
        };

        err = ftWalk(root, &cfg);
        ASSERT_TRUE("no err", err == OK);

        ASSERT_TRUE("root dir",
                    strcmp(root->root_dir, "tests/test_tree/a") == 0);
        ASSERT_TRUE("root path", strcmp(root->path, "") == 0);
        ASSERT_TRUE("root is_dir", root->is_dir == 1);

        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(integration)
{
        RUN_TEST(test_walk_tree);
        return NULL;
}
