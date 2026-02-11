#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "strbuf.h"
}

// Helper: verify core invariants (useful to reuse in tests)
static void expect_invariants(const strbuf_t& sb) {
    // c_str must never be null
    ASSERT_NE(strbuf_c_str(&sb), nullptr);

    // If capacity == 0, data may be null; otherwise data must be non-null
    if (sb.capacity == 0) {
        // ok: sb.data may be nullptr
        EXPECT_EQ(sb.size, 0u); // by design in our API
        EXPECT_STREQ(strbuf_c_str(&sb), "");
    } else {
        ASSERT_NE(sb.data, nullptr);
        // size must fit in capacity with space for '\0'
        ASSERT_LE(sb.size + 1, sb.capacity);
        // trailing terminator must be present at logical end
        EXPECT_EQ(sb.data[sb.size], '\0');
    }
}

// Helper to build a repeated pattern string
static std::string repeat(const std::string& s, size_t times) {
    std::string out;
    out.reserve(s.size() * times);
    for (size_t i = 0; i < times; ++i) out += s;
    return out;
}

// Helper to assert core invariants that SHOULD always hold
static void assert_invariants(const strbuf_t& sb) {
    ASSERT_NE(strbuf_c_str(&sb), nullptr);

    if (sb.capacity == 0) {
        // lazy state should behave like empty string
        EXPECT_EQ(sb.size, 0u);
        EXPECT_STREQ(strbuf_c_str(&sb), "");
    } else {
        ASSERT_NE(sb.data, nullptr);
        ASSERT_LE(sb.size + 1, sb.capacity);
        EXPECT_EQ(sb.data[sb.size], '\0');
    }
}

static void assert_basic_invariants(const strbuf_t& sb) {
    ASSERT_NE(strbuf_c_str(&sb), nullptr);
    if (sb.capacity == 0) {
        EXPECT_EQ(sb.size, 0u);
        EXPECT_STREQ(strbuf_c_str(&sb), "");
    } else {
        ASSERT_NE(sb.data, nullptr);
        ASSERT_LE(sb.size + 1, sb.capacity);
        EXPECT_EQ(sb.data[sb.size], '\0');
    }
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

/* Extra tests */

TEST(StrBufExtraTest, AppendEmptyStringDoesNotChangeState) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 3), 0);

    // Start empty
    const size_t cap0 = sb.capacity;
    ASSERT_EQ(strbuf_append_cstr(&sb, ""), 0);
    EXPECT_EQ(sb.size, 0u);
    EXPECT_EQ(sb.capacity, cap0);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    expect_invariants(sb);

    // After non-empty append, appending empty shouldn't change anything
    ASSERT_EQ(strbuf_append_cstr(&sb, "abc"), 0);
    const size_t cap1 = sb.capacity;
    const size_t size1 = sb.size;
    ASSERT_EQ(strbuf_append_cstr(&sb, ""), 0);
    EXPECT_EQ(sb.size, size1);
    EXPECT_EQ(sb.capacity, cap1);
    EXPECT_STREQ(strbuf_c_str(&sb), "abc");
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, AppendCStringToLazyInitAllocatesAndKeepsInvariant) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);
    expect_invariants(sb);

    ASSERT_EQ(strbuf_append_cstr(&sb, "a"), 0);
    EXPECT_EQ(sb.size, 1u);
    EXPECT_STREQ(strbuf_c_str(&sb), "a");
    EXPECT_GE(sb.capacity, sb.size + 1);
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, ClearOnLazyInitKeepsEmptyAndSafe) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);
    strbuf_clear(&sb);
    EXPECT_EQ(sb.size, 0u);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, ClearAfterEmbeddedNullDataPreservesCapacityAndResetsTerminator) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    const char raw[] = {'x','\0','y'};
    ASSERT_EQ(strbuf_append_n(&sb, raw, sizeof(raw)), 0);
    ASSERT_EQ(sb.size, sizeof(raw));
    const size_t cap_before = sb.capacity;

    strbuf_clear(&sb);
    EXPECT_EQ(sb.size, 0u);
    EXPECT_EQ(sb.capacity, cap_before);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    if (sb.capacity > 0 && sb.data) {
        EXPECT_EQ(sb.data[0], '\0');
    }
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, AppendNZeroBytesIsNoOpButKeepsInvariant) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    ASSERT_EQ(strbuf_append_cstr(&sb, "hi"), 0);
    const size_t cap_before = sb.capacity;
    const size_t size_before = sb.size;

    ASSERT_EQ(strbuf_append_n(&sb, "whatever", 0), 0);
    EXPECT_EQ(sb.size, size_before);
    EXPECT_EQ(sb.capacity, cap_before);
    EXPECT_STREQ(strbuf_c_str(&sb), "hi");
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, AppendNNullPointerWithZeroLengthIsOkButNonZeroIsError) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    // Common robust API behavior: (NULL, 0) is OK
    EXPECT_EQ(strbuf_append_n(&sb, nullptr, 0), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    expect_invariants(sb);

    // (NULL, >0) must fail
    EXPECT_EQ(strbuf_append_n(&sb, nullptr, 1), -1);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, ManySmallAppendsMatchExpectedString) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    std::string expected;
    for (int i = 0; i < 200; ++i) {
        ASSERT_EQ(strbuf_append_cstr(&sb, "a"), 0);
        expected.push_back('a');
        EXPECT_EQ(sb.size, expected.size());
        // Compare with expected
        EXPECT_EQ(std::string(strbuf_c_str(&sb)), expected);
        expect_invariants(sb);
    }

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, CapacityNeverLessThanSizePlusOneAfterGrowth) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    // Create a long string
    std::string big(1000, 'z');
    ASSERT_EQ(strbuf_append_cstr(&sb, big.c_str()), 0);

    EXPECT_GE(sb.capacity, sb.size + 1);
    EXPECT_EQ(sb.size, big.size());
    // Sanity: c_str equals big
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), big);
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, AppendAfterFreeReinitWorks) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "x"), 0);
    strbuf_free(&sb);

    // Re-init should restore to a valid empty state
    ASSERT_EQ(strbuf_init(&sb, 2), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "ok"), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "ok");
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufExtraTest, AppendNThenAppendCStrKeepsTrailingNullAtLogicalEnd) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    const char raw[] = {'A','\0','B'};
    ASSERT_EQ(strbuf_append_n(&sb, raw, sizeof(raw)), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "ZZ"), 0);

    // size should be raw bytes + 2
    EXPECT_EQ(sb.size, sizeof(raw) + 2);
    // trailing terminator must be at sb.data[sb.size]
    ASSERT_NE(sb.data, nullptr);
    EXPECT_EQ(sb.data[sb.size], '\0');

    // c_str still stops at first embedded null, so it should be "A"
    EXPECT_STREQ(strbuf_c_str(&sb), "A");

    // But raw bytes + suffix must be present in memory
    EXPECT_EQ(sb.data[0], 'A');
    EXPECT_EQ(sb.data[1], '\0');
    EXPECT_EQ(sb.data[2], 'B');
    EXPECT_EQ(sb.data[3], 'Z');
    EXPECT_EQ(sb.data[4], 'Z');

    expect_invariants(sb);
    strbuf_free(&sb);
}

TEST(StrBufExtraTest, InitWithCapacityOneMustBeAbleToAppendSingleChar) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    // Should grow because "A" requires 2 bytes including '\0'
    ASSERT_EQ(strbuf_append_cstr(&sb, "A"), 0);
    EXPECT_EQ(sb.size, 1u);
    EXPECT_STREQ(strbuf_c_str(&sb), "A");
    EXPECT_GE(sb.capacity, 2u);
    expect_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, CStrPointerIsStableAcrossNoOpOperations) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);

    const char* p0 = strbuf_c_str(&sb);
    strbuf_clear(&sb);
    const char* p1 = strbuf_c_str(&sb);

    EXPECT_NE(p0, nullptr);
    EXPECT_NE(p1, nullptr);
    EXPECT_STREQ(p0, "");
    EXPECT_STREQ(p1, "");

    // It's okay if pointer changes (e.g., from static "" to allocated),
    // but must remain a valid C-string and invariants must hold.
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendCStrLargeExactContent) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    std::string big(4096, 'x');
    ASSERT_EQ(strbuf_append_cstr(&sb, big.c_str()), 0);
    EXPECT_EQ(sb.size, big.size());
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), big);
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendCStrManyChunksMatchesExpected) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 2), 0);

    std::vector<std::string> chunks = {"ab", "c", "def", "", "ghij", "k"};
    std::string expected;
    for (const auto& ch : chunks) {
        ASSERT_EQ(strbuf_append_cstr(&sb, ch.c_str()), 0);
        expected += ch;
        EXPECT_EQ(sb.size, expected.size());
        EXPECT_EQ(std::string(strbuf_c_str(&sb)), expected);
        assert_invariants(sb);
    }

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendCStrNullIsErrorAndStateUnchanged) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "hi"), 0);

    const size_t size_before = sb.size;
    const size_t cap_before = sb.capacity;
    std::string before = strbuf_c_str(&sb);

    EXPECT_EQ(strbuf_append_cstr(&sb, nullptr), -1);

    EXPECT_EQ(sb.size, size_before);
    EXPECT_EQ(sb.capacity, cap_before);
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), before);
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendNEmbeddedNullAndThenMoreData) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    const char raw[] = {'A','\0','B','C'};
    ASSERT_EQ(strbuf_append_n(&sb, raw, sizeof(raw)), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "ZZ"), 0);

    EXPECT_EQ(sb.size, sizeof(raw) + 2);
    ASSERT_NE(sb.data, nullptr);
    EXPECT_EQ(sb.data[sb.size], '\0'); // terminator at logical end
    EXPECT_STREQ(strbuf_c_str(&sb), "A"); // C-string stops at embedded NUL

    // Verify raw bytes + suffix are preserved in memory
    EXPECT_EQ(sb.data[0], 'A');
    EXPECT_EQ(sb.data[1], '\0');
    EXPECT_EQ(sb.data[2], 'B');
    EXPECT_EQ(sb.data[3], 'C');
    EXPECT_EQ(sb.data[4], 'Z');
    EXPECT_EQ(sb.data[5], 'Z');

    assert_invariants(sb);
    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendNZeroLengthIsNoOp) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 3), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "abc"), 0);

    const size_t size_before = sb.size;
    const size_t cap_before  = sb.capacity;
    std::string before = strbuf_c_str(&sb);

    EXPECT_EQ(strbuf_append_n(&sb, "whatever", 0), 0);

    EXPECT_EQ(sb.size, size_before);
    EXPECT_EQ(sb.capacity, cap_before);
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), before);
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendNNullPointerZeroLengthOkNonZeroError) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);

    EXPECT_EQ(strbuf_append_n(&sb, nullptr, 0), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "");

    EXPECT_EQ(strbuf_append_n(&sb, nullptr, 1), -1);
    EXPECT_STREQ(strbuf_c_str(&sb), "");

    assert_invariants(sb);
    strbuf_free(&sb);
}

TEST(StrBufHardTest, ResizeBehaviorViaClearAndAppendKeepsCapacity) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 8), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "1234567"), 0);
    const size_t cap_before = sb.capacity;

    strbuf_clear(&sb);
    EXPECT_EQ(sb.size, 0u);
    EXPECT_EQ(sb.capacity, cap_before);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    assert_invariants(sb);

    ASSERT_EQ(strbuf_append_cstr(&sb, "x"), 0);
    EXPECT_EQ(sb.size, 1u);
    EXPECT_GE(sb.capacity, cap_before); // should not shrink
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, TerminatorAlwaysPresentAfterEachAppend) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    // Sequence of mixed appends
    ASSERT_EQ(strbuf_append_cstr(&sb, "a"), 0);
    if (sb.capacity > 0) ASSERT_NE(sb.data, nullptr);
    assert_invariants(sb);

    const char raw1[] = {'b','c'};
    ASSERT_EQ(strbuf_append_n(&sb, raw1, sizeof(raw1)), 0);
    assert_invariants(sb);

    ASSERT_EQ(strbuf_append_cstr(&sb, "def"), 0);
    assert_invariants(sb);

    // Sanity: content as c_str (no embedded NULs in this case)
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), "abcdef");

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendCStrDoesNotReadBeyondSuffix) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    // Provide a buffer that contains '\0' early; append_cstr should stop at first '\0'
    const char tricky[] = {'h','i','\0','X','Y','Z'};
    ASSERT_EQ(strbuf_append_cstr(&sb, tricky), 0);

    EXPECT_EQ(sb.size, 2u);
    EXPECT_STREQ(strbuf_c_str(&sb), "hi");
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufHardTest, StressAlternatingClearAndAppend) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 2), 0);

    for (int i = 0; i < 200; ++i) {
        ASSERT_EQ(strbuf_append_cstr(&sb, "hello"), 0);
        ASSERT_EQ(std::string(strbuf_c_str(&sb)), "hello");

        strbuf_clear(&sb);
        ASSERT_EQ(sb.size, 0u);
        ASSERT_STREQ(strbuf_c_str(&sb), "");
        assert_invariants(sb);
    }

    strbuf_free(&sb);
}

TEST(StrBufHardTest, AppendCStrHugeRepeatedPattern) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);

    std::string pat = "0123456789";
    std::string expected = repeat(pat, 200); // 2000 chars
    ASSERT_EQ(strbuf_append_cstr(&sb, expected.c_str()), 0);

    EXPECT_EQ(sb.size, expected.size());
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), expected);
    assert_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, CStrAfterInitWithCapacityReturnsWritableEmptyString) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    EXPECT_NE(strbuf_c_str(&sb), nullptr);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    // Invariant: data exists when capacity > 0 and is NUL-terminated at size
    ASSERT_NE(sb.data, nullptr);
    EXPECT_EQ(sb.data[0], '\0');
    assert_basic_invariants(sb);
    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, AppendEmptyCStringIsNoOpSuccess) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    ASSERT_NE(sb.data, nullptr);
    const size_t cap0 = sb.capacity;
    const size_t size0 = sb.size;

    EXPECT_EQ(strbuf_append_cstr(&sb, ""), 0);
    EXPECT_EQ(sb.capacity, cap0);
    EXPECT_EQ(sb.size, size0);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, AppendCStringOnLazyInitWorks) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.capacity, 0u);
    EXPECT_EQ(sb.size, 0u);

    EXPECT_EQ(strbuf_append_cstr(&sb, "A"), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "A");
    EXPECT_EQ(sb.size, 1u);
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, AppendNZeroLengthIsOkAndNoOp) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "hi"), 0);

    const size_t cap0 = sb.capacity;
    const size_t size0 = sb.size;
    std::string before = strbuf_c_str(&sb);

    EXPECT_EQ(strbuf_append_n(&sb, "ignored", 0), 0);
    EXPECT_EQ(sb.capacity, cap0);
    EXPECT_EQ(sb.size, size0);
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), before);
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, AppendNNullPointerZeroLengthIsOkNonZeroIsError) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 2), 0);

    EXPECT_EQ(strbuf_append_n(&sb, nullptr, 0), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    assert_basic_invariants(sb);

    EXPECT_EQ(strbuf_append_n(&sb, nullptr, 1), -1);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, GrowAllocatesEnoughForTerminatorAndCopiesAllBytes) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 2), 0);

    // Fill to force growth (need room for '\0')
    ASSERT_EQ(strbuf_append_cstr(&sb, "A"), 0);
    ASSERT_EQ(std::string(strbuf_c_str(&sb)), "A");

    // This requires growth from capacity 2
    ASSERT_EQ(strbuf_append_cstr(&sb, "BCDE"), 0);
    EXPECT_EQ(std::string(strbuf_c_str(&sb)), "ABCDE");

    // Must have enough capacity for size+1, and terminator present
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, ClearOnLazyInitIsSafe) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);

    // Must be safe and keep c_str non-null/empty
    strbuf_clear(&sb);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.size, 0u);
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, ClearAfterFreeIsSafe) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "x"), 0);
    strbuf_free(&sb);

    // Must be safe even if user calls clear after free
    strbuf_clear(&sb);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.size, 0u);
    assert_basic_invariants(sb);

    strbuf_free(&sb); // idempotent
}

TEST(StrBufBugCatcher, AppendAfterClearFromNonEmptyKeepsWorking) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 4), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "test"), 0);

    strbuf_clear(&sb);
    EXPECT_STREQ(strbuf_c_str(&sb), "");
    EXPECT_EQ(sb.size, 0u);
    assert_basic_invariants(sb);

    ASSERT_EQ(strbuf_append_cstr(&sb, "ok"), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "ok");
    EXPECT_EQ(sb.size, 2u);
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, AppendNOnLazyInitWorks) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 0), 0);

    const char raw[] = {'A','B','C'};
    ASSERT_EQ(strbuf_append_n(&sb, raw, sizeof(raw)), 0);
    EXPECT_EQ(sb.size, 3u);
    ASSERT_NE(sb.data, nullptr);
    EXPECT_EQ(sb.data[0], 'A');
    EXPECT_EQ(sb.data[1], 'B');
    EXPECT_EQ(sb.data[2], 'C');
    EXPECT_EQ(sb.data[sb.size], '\0');
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, RepeatedSmallAppendsBuildCorrectString) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);

    std::string expected;
    for (int i = 0; i < 50; ++i) {
        ASSERT_EQ(strbuf_append_cstr(&sb, "a"), 0);
        expected.push_back('a');
        EXPECT_EQ(sb.size, expected.size());
        EXPECT_EQ(std::string(strbuf_c_str(&sb)), expected);
        assert_basic_invariants(sb);
    }

    strbuf_free(&sb);
}

TEST(StrBufBugCatcher, AppendNPreservesEmbeddedNullAndStillTerminatesAtEnd) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 3), 0);

    const char raw[] = {'X','\0','Y','Z'};
    ASSERT_EQ(strbuf_append_n(&sb, raw, sizeof(raw)), 0);

    EXPECT_EQ(sb.size, sizeof(raw));
    ASSERT_NE(sb.data, nullptr);
    EXPECT_EQ(sb.data[0], 'X');
    EXPECT_EQ(sb.data[1], '\0');
    EXPECT_EQ(sb.data[2], 'Y');
    EXPECT_EQ(sb.data[3], 'Z');
    EXPECT_EQ(sb.data[sb.size], '\0');
    // c_str() shows only up to first embedded null
    EXPECT_STREQ(strbuf_c_str(&sb), "X");
    assert_basic_invariants(sb);

    strbuf_free(&sb);
}

// Optional "robustness" tests for invalid init
TEST(StrBufBugCatcher, InitWithCapacityOneThenAppendSingleCharSucceeds) {
    strbuf_t sb;
    ASSERT_EQ(strbuf_init(&sb, 1), 0);
    ASSERT_EQ(strbuf_append_cstr(&sb, "A"), 0);
    EXPECT_STREQ(strbuf_c_str(&sb), "A");
    EXPECT_EQ(sb.size, 1u);
    assert_basic_invariants(sb);
    strbuf_free(&sb);
}
