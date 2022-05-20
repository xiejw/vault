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
test_empty_roots()
{
        error_t err;

        vec_t(struct hlog *) logs = vecNew();
        struct hlog a             = ENTRY_ADD("foo/bar/abc");
        struct hlog b             = ENTRY_ADD("foo/bar/def");
        struct hlog c             = ENTRY_ADD("bar/def");
        struct hlog d             = ENTRY_ADD("abc");
        vecPushBack(&logs, &a);
        vecPushBack(&logs, &b);
        vecPushBack(&logs, &c);
        vecPushBack(&logs, &d);

        struct ft_node *root;
        err = hlogToFt("/root_dir", logs, &root);

        ASSERT_TRUE("no err", err == OK);

        sds_t s = sdsEmpty();
        ftDumpSds(&s, root);
        printf("tree:\n%s", s);
        sdsFree(s);

        vecFree(logs);
        return NULL;
}

DECLARE_TEST_SUITE(hlog)
{
        RUN_TEST(test_empty_roots);
        return NULL;
}
