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
        root->root_dir           = root_dir;
        root->path               = sdsNew("a");
        root->is_dir             = 1;

        struct ft_node *node_file = ftNodeNew();
        root->root_dir            = root_dir;
        root->path                = sdsNew("b");
        root->is_dir              = 0;

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
test_print_tree()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();

        err = ftVisit(print_tree_fn, &s, root, FTV_PREORDER);

        ASSERT_TRUE("no err", err == OK);

        printf("output:\n%s", s);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(ft_visit)
{
        RUN_TEST(test_print_tree);
        return NULL;
}
