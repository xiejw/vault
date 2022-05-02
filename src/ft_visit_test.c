#include "testing/testing.h"

// eva
#include <adt/sds.h>
#include <base/fpath.h>

// vault
#include "ft_visit.h"

static struct ft_node *
buildTree()
{
        // root ->
        //     a (+)
        //     b
        sds_t root_dir = sdsNew("/root");

        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmpty();
        root->is_dir         = 1;

        struct ft_node *node_dir = ftNodeNew();
        node_dir->parent         = root;
        node_dir->root_dir       = root_dir;
        node_dir->path           = sdsNew("a");
        node_dir->is_dir         = 1;

        struct ft_node *node_file = ftNodeNew();
        node_file->parent         = root;
        node_file->root_dir       = root_dir;
        node_file->path           = sdsNew("b");
        node_file->is_dir         = 0;

        vecPushBack(&root->children, node_dir);
        vecPushBack(&root->children, node_file);

        return root;
}

static error_t
print_tree_fn(void *data, struct ft_node *node, _out_ int *outflag)
{
        sds_t *s   = (sds_t *)data;
        sds_t path = fpJoinSds(node->root_dir, node->path);
        sdsCatPrintf(s, "%s\n", path);
        sdsFree(path);
        *outflag = FTV_NO_CHANGE;
        return OK;
}

static char *
test_print_tree_preorder()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_PREORDER);

        const char *expected = "/root\n/root/a\n/root/b\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_postorder()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_POSTORDER);

        const char *expected = "/root/a\n/root/b\n/root\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(ft_visit)
{
        RUN_TEST(test_print_tree_preorder);
        RUN_TEST(test_print_tree_postorder);
        return NULL;
}
