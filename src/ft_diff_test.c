#include "testing/testing.h"

// vault
#include "ft_diff.h"

static struct ft_node *
buildTreeA()
{
        sds_t root_dir = sdsNew("/rootA");

        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmpty();
        root->is_dir         = 1;
        root->checksum       = sdsNew("#abc");

        return root;
}

static struct ft_node *
buildTreeBWithSum(_moved_in_ sds_t root_check_sum)
{
        sds_t root_dir = sdsNew("/rootB");

        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmpty();
        root->is_dir         = 1;
        root->checksum       = root_check_sum;

        return root;
}

static struct ft_node *
buildTreeB()
{
        return buildTreeBWithSum(sdsNew("#abc"));
}

static char *
test_empty_roots()
{
        struct ft_node *root_a = buildTreeA();
        struct ft_node *root_b = buildTreeB();

        vec_t(struct ft_node *) nodes_diff_in_a = vecNew();
        vec_t(struct ft_node *) nodes_diff_in_b = vecNew();

        ASSERT_TRUE("no error", OK == ftDiff(root_a, root_a, &nodes_diff_in_a,
                                             &nodes_diff_in_b));

        vecFree(nodes_diff_in_a);
        vecFree(nodes_diff_in_b);
        ftFree(root_a);
        ftFree(root_b);
        return NULL;
}

DECLARE_TEST_SUITE(ft_diff)
{
        RUN_TEST(test_empty_roots);
        return NULL;
}
