#include "testing/testing.h"

#include "hlog.h"

// vault
#include "ft_visit.h"

#define ENTRY_ADD(fpath)                                                       \
        {                                                                      \
                .cmd = HLOG_ADD, .timestamp = 123,                             \
                .checksum =                                                    \
                    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b" \
                    "7852b855",                                                \
                .path = sdsNew((fpath)),                                       \
        }

static char *
test_hlogs_to_ft()
{
        error_t err;

        // create hlogs.
        vec_t(struct hlog *) hlogs = vecNew();
        struct hlog a              = ENTRY_ADD("foo/bar/abc");
        struct hlog b              = ENTRY_ADD("foo/bar/def");
        struct hlog c              = ENTRY_ADD("bar/def");
        struct hlog d              = ENTRY_ADD("abc");
        vecPushBack(&hlogs, &a);
        vecPushBack(&hlogs, &b);
        vecPushBack(&hlogs, &c);
        vecPushBack(&hlogs, &d);

        struct ft_node *root;
        sds_t root_dir = sdsNew("/root_dir");  // owned by root later.

        // create ft
        err = hlogToFt(root_dir, hlogs, &root);
        ASSERT_TRUE("no err", err == OK);

        // check tree structure
        {
                sds_t s = sdsEmpty();
                ftDumpSds(&s, root);
                printf("tree:\n%s", s);
                sdsFree(s);
        }

        // free hlogs
        for (size_t i = 0; i < vecSize(hlogs); i++) {
                sdsFree(hlogs[i]->path);
        }
        vecFree(hlogs);

        // free tree
        ftFree(root);
        return NULL;
}

DECLARE_TEST_SUITE(hlog)
{
        RUN_TEST(test_hlogs_to_ft);
        return NULL;
}
