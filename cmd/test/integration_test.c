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
        // expected data:
        //     tests/test_tree/a
        //     tests/test_tree/a/h
        //     tests/test_tree/a/d
        //     tests/test_tree/a/e
        //     tests/test_tree/a/e/f
        //     tests/test_tree/a/e/f/g
        //     tests/test_tree/a/b
        //     tests/test_tree/a/b/c
        struct ft_node *root = ftRootNew(sdsNew("tests/test_tree/a"));

        struct ft_walk_config cfg = {
            .dangling_sym_link = FT_SILENT,  // for testing, we ignore it.
        };

        // walk and expect no issue.
        err = ftWalk(root, &cfg);
        ASSERT_TRUE("no err", err == OK);

        // examine root node.
        ASSERT_TRUE("root dir",
                    strcmp(root->root_dir, "tests/test_tree/a") == 0);
        ASSERT_TRUE("root path", strcmp(root->path, "") == 0);
        ASSERT_TRUE("root is_dir", root->is_dir == 1);
        ASSERT_TRUE("root is_dir", root->is_dir == 1);

        // examine children
        ASSERT_TRUE("root children count", vecSize(root->children) == 4);

        int mask = 0;  // records the bits so we know all children are visited.
        struct ft_node *child;
        for (size_t i = 0; i < vecSize(root->children); i++) {
                child = root->children[i];
                if (strcmp(child->path, "h") == 0) {
                        mask |= 1;
                        continue;
                } else if (strcmp(child->path, "d") == 0) {
                        mask |= 2;
                        continue;
                } else if (strcmp(child->path, "e") == 0) {
                        mask |= 4;
                        continue;
                } else if (strcmp(child->path, "b") == 0) {
                        mask |= 8;
                        continue;
                }
                ASSERT_TRUE("should not reach", 0);
        }

        ASSERT_TRUE("found all children", mask == 15);

        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(integration)
{
        RUN_TEST(test_walk_tree);
        return NULL;
}
