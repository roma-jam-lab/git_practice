#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "strbuf.h"
}

TEST(StrBufTest, InitZeroCapacityIsValidAndCStrIsNeverNull) {
    strbuf_t sb;
    EXPECT_EQ(strbuf_init(&sb, 0), 0);
    EXPECT_NE(strbuf_c_str(&sb), nullptr);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.size, 0u);
    EXPECT_EQ(sb.capacity, 0u);
    strbuf_free(&sb);
}

TEST(StrBufTest, InitWithCapacityAllocatesAndIsEmpty) {
    strbuf_t sb;
    EXPECT_EQ(strbuf_init(&sb, 8), 0);
    EXPECT_NE(strbuf_c_str(&sb), nullptr);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.size, 0u);
    EXPECT_GE(sb.capacity, 1u); // must at least hold '\0'
    strbuf_free(&sb);
}

TEST(StrBufTest, AppendCStrBasic) {
    strbuf_t sb;
    strbuf_init(&sb, 1);

    EXPECT_EQ(strbuf_append_cstr(&sb, "hi"), 0);
    EXPECT_EQ(sb.size, 2u);
    EXPECT_STREQ(strbuf_c_str(&sb), "hi");

    EXPECT_EQ(strbuf_append_cstr(&sb, " there"), 0);
    EXPECT_EQ(sb.size, 8u);
    EXPECT_STREQ(strbuf_c_str(&sb), "hi there");

    strbuf_free(&sb);
}

TEST(StrBufTest, AppendTriggersGrowthAndPreservesData) {
    strbuf_t sb;
    strbuf_init(&sb, 2);

    EXPECT_EQ(strbuf_append_cstr(&sb, "abcdef"), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "abcdef");
    EXPECT_EQ(sb.size, 6u);
    EXPECT_GE(sb.capacity, sb.size + 1); // +1 for '\0'

    EXPECT_EQ(strbuf_append_cstr(&sb, "XYZ"), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "abcdefXYZ");
    EXPECT_EQ(sb.size, 9u);

    strbuf_free(&sb);
}

TEST(StrBufTest, AppendNAllowsEmbeddedNullBytesButCStrStopsAtFirstNull) {
    strbuf_t sb;
    strbuf_init(&sb, 1);

    const char raw[] = {'A','\0','B','C'};
    EXPECT_EQ(strbuf_append_n(&sb, raw, sizeof(raw)), 0);

    EXPECT_EQ(sb.size, sizeof(raw));
    // c_str() is C-string; it will appear as "A"
    EXPECT_STREQ(strbuf_c_str(&sb), "A");
    // But raw bytes must still be stored
    ASSERT_GE(sb.capacity, sb.size + 1);
    EXPECT_EQ(sb.data[0], 'A');
    EXPECT_EQ(sb.data[1], '\0');
    EXPECT_EQ(sb.data[2], 'B');
    EXPECT_EQ(sb.data[3], 'C');
    EXPECT_EQ(sb.data[sb.size], '\0'); // invariant: trailing terminator

    strbuf_free(&sb);
}

TEST(StrBufTest, ClearResetsToEmptyButKeepsCapacity) {
    strbuf_t sb;
    strbuf_init(&sb, 4);
    strbuf_append_cstr(&sb, "test");
    const size_t old_cap = sb.capacity;

    strbuf_clear(&sb);
    EXPECT_EQ(sb.size, 0u);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.capacity, old_cap);

    strbuf_free(&sb);
}

TEST(StrBufTest, AppendNullSuffixIsError) {
    strbuf_t sb;
    strbuf_init(&sb, 1);
    EXPECT_EQ(strbuf_append_cstr(&sb, nullptr), -1);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    strbuf_free(&sb);
}

TEST(StrBufTest, FreeIsIdempotent) {
    strbuf_t sb;
    strbuf_init(&sb, 2);
    strbuf_append_cstr(&sb, "x");

    strbuf_free(&sb);
    // second free must be safe
    strbuf_free(&sb);

    // c_str must still be non-null and empty even after free
    EXPECT_NE(strbuf_c_str(&sb), nullptr);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
}
