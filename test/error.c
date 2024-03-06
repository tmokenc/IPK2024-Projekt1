#include "../lib/greatest.h"
#include "../src/error.h"

SUITE(error);

TEST set(void) {
    ASSERT_EQ(get_error(), Error_None);
    ASSERT_FALSE(get_error());

    set_error(Error_InvalidArgument);
    ASSERT_EQ(get_error(), Error_InvalidArgument);
    ASSERT_FALSE(!get_error());

    set_error(Error_InvalidInput);
    ASSERT_EQ(get_error(), Error_InvalidInput);
    ASSERT_FALSE(!get_error());

    PASS();
}

GREATEST_SUITE(error) {
    RUN_TEST(set);
}
