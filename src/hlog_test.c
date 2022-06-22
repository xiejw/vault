#include "testing/testing.h"

#include "hlog.h"

// vault
#include "ft_visit.h"

// -----------------------------------------------------------------------------
// test helpers
// -----------------------------------------------------------------------------

#define ENTRY_ADD(hlogs, fpath)                                                \
        do {                                                                   \
                struct hlog *t = malloc(sizeof(*t));                           \
                struct hlog t1 = {                                             \
                    .cmd       = HLOG_ADD,                                     \
                    .timestamp = 123,                                          \
                    .checksum =                                                \
                        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495" \
                        "991b"                                                 \
                        "7852b855",                                            \
                    .path = sdsNew((fpath)),                                   \
                };                                                             \
                *t = t1;                                                       \
                vecPushBack(&(hlogs), t);                                      \
        } while (0)

static void
freeHlogs(vec_t(struct hlog *) hlogs)
{
        // free hlogs
        for (size_t i = 0; i < vecSize(hlogs); i++) {
                sdsFree(hlogs[i]->path);
                free(hlogs[i]);
        }
        vecFree(hlogs);
}

// -----------------------------------------------------------------------------
// unit tests for hlogToFt
// -----------------------------------------------------------------------------
static char *
test_hlogs_to_ft()
{
        error_t err;
        sds_t s = sdsEmpty();

        // create hlogs.
        vec_t(struct hlog *) hlogs = vecNew();
        ENTRY_ADD(hlogs, "foo/bar/abc");
        ENTRY_ADD(hlogs, "foo/bar/def");
        ENTRY_ADD(hlogs, "foo/xyz");
        ENTRY_ADD(hlogs, "bar/def");
        ENTRY_ADD(hlogs, "abc");

        struct ft_node *root;
        sds_t root_dir = sdsNew("/root_dir");  // owned by root later.

        // create ft
        err = hlogToFt(root_dir, hlogs, &root);
        ASSERT_TRUE("no err", err == OK);

        // check tree structure
        ftDumpSds(&s, root);

        char *expected =
            "root - /root_dir\n"
            "    +-> foo (+)\n"
            "        +-> foo/bar (+)\n"
            "            +-> foo/bar/abc\n"
            "            +-> foo/bar/def\n"
            "        +-> foo/xyz\n"
            "    +-> bar (+)\n"
            "        +-> bar/def\n"
            "    +-> abc\n";
        ASSERT_TRUE("dump", 0 == strcmp(expected, s));

        // free
        sdsFree(s);
        freeHlogs(hlogs);
        ftFree(root);
        return NULL;
}

// -----------------------------------------------------------------------------
// unit tests for hlogFromSds
// -----------------------------------------------------------------------------
static char *
test_hlogs_from_empty_sds()
{
        sds_t s                    = sdsEmpty();
        vec_t(struct hlog *) hlogs = vecNew();

        ASSERT_TRUE("no err", OK == hlogFromSds(s, &hlogs));
        ASSERT_TRUE("no change", 0 == vecSize(hlogs));

        vecFree(hlogs);
        sdsFree(s);
        return NULL;
}

static char *
test_hlogs_fail_with_newline_only()
{
        sds_t s                    = sdsNew("\n");
        vec_t(struct hlog *) hlogs = vecNew();

        ASSERT_TRUE("expect err", OK != hlogFromSds(s, &hlogs));

        errFree();
        vecFree(hlogs);
        sdsFree(s);
        return NULL;
}

static char *
test_hlogs_from_oneline()
{
        sds_t s                    = sdsNew("+\n");
        vec_t(struct hlog *) hlogs = vecNew();
        ASSERT_TRUE("no err", OK == hlogFromSds(s, &hlogs));
        ASSERT_TRUE("size", 1 == vecSize(hlogs));
        ASSERT_TRUE("op", 1 == hlogs[0]->cmd);

        freeHlogs(hlogs);
        sdsFree(s);
        return NULL;
}

static char *
test_hlogs_fail_with_invalid_string()
{
        sds_t s                    = sdsNew("not working");
        vec_t(struct hlog *) hlogs = vecNew();

        ASSERT_TRUE("err unsupported", OK != hlogFromSds(s, &hlogs));

        errFree();
        vecFree(hlogs);
        sdsFree(s);
        return NULL;
}

DECLARE_TEST_SUITE(hlog)
{
        RUN_TEST(test_hlogs_to_ft);
        RUN_TEST(test_hlogs_from_empty_sds);
        RUN_TEST(test_hlogs_fail_with_newline_only);
        RUN_TEST(test_hlogs_from_oneline);
        RUN_TEST(test_hlogs_fail_with_invalid_string);
        return NULL;
}
