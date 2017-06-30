#include <gtest/gtest.h>

extern "C" {
#include <buffer.h>
}

static char eight_nulls[] = "\x00\x00\x00\x00\x00\x00\x00\x00";

TEST(buffer, zero_size_init) {
    int i;
    int res = 0;
    struct sylkie_buffer* buf = sylkie_buffer_init(0);

    res = sylkie_buffer_add(buf, eight_nulls, sizeof(eight_nulls));

    EXPECT_EQ(res, 0);

    for (i = 0; i < buf->len; ++i) {
        EXPECT_EQ(buf->data[i], 0x00);
    }
    EXPECT_EQ(i, 9);

    sylkie_buffer_free(buf);
}

TEST(buffer, null_add) {
    int res = 0;
    struct sylkie_buffer* buf = NULL;

    res = sylkie_buffer_add(buf, eight_nulls, sizeof(eight_nulls));

    EXPECT_EQ(res, -1);

    sylkie_buffer_free(buf);
}
