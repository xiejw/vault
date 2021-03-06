#include "testing/testing.h"

// eva
#include <adt/sds.h>
#include <base/fpath.h>

// vault
#include "ft.h"

// -----------------------------------------------------------------------------
// helper methods for building trees
// -----------------------------------------------------------------------------

static struct ft_node *
buildTree()
{
        // root ->
        //     a (+)
        //     b
        sds_t root_dir = sdsNew("/root/");

        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmpty();
        root->is_dir         = 1;

        struct ft_node *node_dir = ftNodeNew();
        node_dir->parent         = root;
        node_dir->root_dir       = root_dir;
        node_dir->path           = sdsNew("a/");
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

// good for sorting:
static struct ft_node *
buildTreeWithTwoLayers()
{
        // root ->
        //     z (+)
        //       +- d
        //       +- c
        //     y (+)
        //     b
        //     a
        sds_t root_dir = sdsNew("/root/");

        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmpty();
        root->is_dir         = 1;

        struct ft_node *node_dir = ftNodeNew();
        node_dir->parent         = root;
        node_dir->root_dir       = root_dir;
        node_dir->path           = sdsNew("z");
        node_dir->is_dir         = 1;

        vecPushBack(&root->children, node_dir);

        {
                struct ft_node *node_file = ftNodeNew();
                node_file->parent         = root;
                node_file->root_dir       = root_dir;
                node_file->path           = sdsNew("d");
                node_file->is_dir         = 0;

                vecPushBack(&node_dir->children, node_file);

                node_file           = ftNodeNew();
                node_file->parent   = root;
                node_file->root_dir = root_dir;
                node_file->path     = sdsNew("c");
                node_file->is_dir   = 0;

                vecPushBack(&node_dir->children, node_file);
        }

        node_dir           = ftNodeNew();
        node_dir->parent   = root;
        node_dir->root_dir = root_dir;
        node_dir->path     = sdsNew("y");
        node_dir->is_dir   = 1;

        vecPushBack(&root->children, node_dir);

        struct ft_node *node_file = ftNodeNew();
        node_file->parent         = root;
        node_file->root_dir       = root_dir;
        node_file->path           = sdsNew("b");
        node_file->is_dir         = 0;

        vecPushBack(&root->children, node_file);

        node_file           = ftNodeNew();
        node_file->parent   = root;
        node_file->root_dir = root_dir;
        node_file->path     = sdsNew("a");
        node_file->is_dir   = 0;

        vecPushBack(&root->children, node_file);

        return root;
}

// -----------------------------------------------------------------------------
// helper methods for visit fns
// -----------------------------------------------------------------------------

static error_t
print_tree_fn(void *data, struct ft_node *node, _out_ int *flag)
{
        sds_t *s   = (sds_t *)data;
        sds_t path = fpJoinSds(node->root_dir, node->path);
        sdsCatPrintf(s, "%s\n", path);
        sdsFree(path);
        *flag = FTV_NO_CHANGE;
        return OK;
}

static error_t
print_tree_fn_outflagcheck(void *data, struct ft_node *node, _out_ int *flag)
{
        if ((*flag) == 1) {
                *flag = FTV_NO_CHANGE;
                return OK;
        }

        sds_t *s   = (sds_t *)data;
        sds_t path = fpJoinSds(node->root_dir, node->path);
        sdsCatPrintf(s, "%s\n", path);
        sdsFree(path);
        *flag = FTV_NO_CHANGE;
        return OK;
}

static error_t
detach_all_fn(void *data, struct ft_node *node, _out_ int *flag)
{
        *flag = FTV_DETACH;
        return OK;
}

// -----------------------------------------------------------------------------
// tests for visit
// -----------------------------------------------------------------------------

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

static char *
test_print_tree_bothorder()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_BOTHORDER);

        const char *expected = "/root\n/root/a\n/root/a\n/root/b\n/root\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_preorder_dironly()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_PREORDER | FTV_DIRONLY);

        const char *expected = "/root\n/root/a\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_postorder_dironly()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_POSTORDER | FTV_DIRONLY);

        const char *expected = "/root/a\n/root\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_bothorder_dironly()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_BOTHORDER | FTV_DIRONLY);

        const char *expected = "/root\n/root/a\n/root/a\n/root\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_preorder_fileonly()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn, &s, root, FTV_PREORDER | FTV_FILEONLY);

        const char *expected = "/root/b\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_postorder_fileonly()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err = ftVisit(print_tree_fn, &s, root, FTV_POSTORDER | FTV_FILEONLY);

        const char *expected = "/root/b\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_bothorder_fileonly()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err = ftVisit(print_tree_fn, &s, root, FTV_BOTHORDER | FTV_FILEONLY);

        const char *expected = "/root/b\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_print_tree_bothorder_outflagcheck()
{
        error_t err;
        struct ft_node *root = buildTree();

        sds_t s = sdsEmpty();
        err     = ftVisit(print_tree_fn_outflagcheck, &s, root, FTV_BOTHORDER);

        const char *expected = "/root\n/root/a\n/root/b\n";

        ASSERT_TRUE("no err", err == OK);
        ASSERT_TRUE("output", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_detach_preorder()
{
        error_t err;
        struct ft_node *root = buildTree();
        err                  = ftVisit(detach_all_fn, NULL, root, FTV_PREORDER);
        ASSERT_TRUE("no err", err == OK);
        // detach will free root.
        return NULL;
}

static char *
test_detach_postorder()
{
        error_t err;
        struct ft_node *root = buildTree();
        err = ftVisit(detach_all_fn, NULL, root, FTV_POSTORDER);
        ASSERT_TRUE("no err", err == OK);
        // detach will free root.
        return NULL;
}

static char *
test_detach_bothorder()
{
        error_t err;
        struct ft_node *root = buildTree();
        err = ftVisit(detach_all_fn, NULL, root, FTV_BOTHORDER);
        ASSERT_TRUE("no err", err == OK);
        // detach will free root.
        return NULL;
}

// -----------------------------------------------------------------------------
// tests for supporting fns
// -----------------------------------------------------------------------------

static char *
test_dump_tree()
{
        struct ft_node *root = buildTree();
        sds_t s              = sdsEmpty();
        ftDumpSds(&s, root);
        const char *expected =
            "root - /root/\n"
            "    +-> a/ (+)\n"
            "    +-> b\n";
        ASSERT_TRUE("check dump", strcmp(expected, s) == 0);
        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_sort_tree()
{
        struct ft_node *root = buildTreeWithTwoLayers();
        sds_t s              = sdsEmpty();

        ftDumpSds(&s, root);
        const char *expected =
            "root - /root/\n"
            "    +-> z (+)\n"
            "        +-> d\n"
            "        +-> c\n"
            "    +-> y (+)\n"
            "    +-> b\n"
            "    +-> a\n";
        ASSERT_TRUE("check dump", strcmp(expected, s) == 0);
        sdsClear(s);

        ftSort(root);
        ftDumpSds(&s, root);

        expected =
            "root - /root/\n"
            "    +-> y (+)\n"
            "    +-> z (+)\n"
            "        +-> c\n"
            "        +-> d\n"
            "    +-> a\n"
            "    +-> b\n";

        ASSERT_TRUE("check dump again", strcmp(expected, s) == 0);

        sdsFree(s);
        ftFree(root);
        return NULL;
}

static char *
test_trim_empty_dir()
{
        struct ft_node *root = buildTree();
        sds_t s              = sdsEmpty();

        ftTrimEmptyDir(root);
        ftDumpSds(&s, root);
        const char *expected =
            "root - /root/\n"
            "    +-> b\n";
        ASSERT_TRUE("check dump", strcmp(expected, s) == 0);
        sdsFree(s);
        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(ft_visit)
{
        // visit
        RUN_TEST(test_print_tree_preorder);
        RUN_TEST(test_print_tree_postorder);
        RUN_TEST(test_print_tree_bothorder);
        RUN_TEST(test_print_tree_preorder_dironly);
        RUN_TEST(test_print_tree_postorder_dironly);
        RUN_TEST(test_print_tree_bothorder_dironly);
        RUN_TEST(test_print_tree_preorder_fileonly);
        RUN_TEST(test_print_tree_postorder_fileonly);
        RUN_TEST(test_print_tree_bothorder_fileonly);
        RUN_TEST(test_print_tree_bothorder_outflagcheck);
        RUN_TEST(test_detach_preorder);
        RUN_TEST(test_detach_postorder);
        RUN_TEST(test_detach_bothorder);

        // supporting fns
        RUN_TEST(test_dump_tree);
        RUN_TEST(test_sort_tree);
        RUN_TEST(test_trim_empty_dir);
        return NULL;
}
