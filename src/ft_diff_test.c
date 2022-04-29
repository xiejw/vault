#include "testing/testing.h"

// vault
#include "ft_diff.h"

static char *
test_empty_trees()
{
        return NULL;
}

DECLARE_TEST_SUITE(diff)
{
        RUN_TEST(test_empty_trees);
        return NULL;
}
