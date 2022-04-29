#include "testing/testing.h"

// vault
#include "ft_visit.h"

static struct ft_node *
buildTree()
{
        sds_t root_dir = sdsNew("/root");

        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmpty();
        root->is_dir         = 1;

        return root;
}

static char *
test_root_only()
{
        struct ft_node *root = buildTree();
        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(ft_visit)
{
        RUN_TEST(test_root_only);
        return NULL;
}
