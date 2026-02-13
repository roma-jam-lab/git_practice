#include <gtest/gtest.h>
#include <vector>

extern "C" {
#include "slist.h"
}

static std::vector<int> to_vec_safe(const slist_t* list, size_t limit = 100000) {
    std::vector<int> out;
    slist_node_t* cur = list->head;
    while (cur && out.size() < limit) {
        out.push_back(cur->value);
        cur = cur->next;
    }
    return out;
}

static void expect_invariants_acyclic(const slist_t& list) {
    // Only call when you believe list is acyclic.
    size_t count = 0;
    slist_node_t* cur = list.head;
    slist_node_t* last = nullptr;
    while (cur) {
        last = cur;
        cur = cur->next;
        ++count;
        ASSERT_LE(count, 200000u);
    }

    EXPECT_EQ(count, list.size);

    if (list.size == 0) {
        EXPECT_EQ(list.head, nullptr);
        EXPECT_EQ(list.tail, nullptr);
    } else {
        EXPECT_NE(list.head, nullptr);
        EXPECT_NE(list.tail, nullptr);
        EXPECT_EQ(list.tail, last);
        EXPECT_EQ(list.tail->next, nullptr);
    }
}

// Helper: build list 1..n
static void build_1_to_n(slist_t* list, int n) {
    for (int i = 1; i <= n; ++i) {
        ASSERT_EQ(slist_push_back(list, i), 0);
    }
}

TEST(SListAlgoTest, ReverseEmptyIsOk) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    EXPECT_EQ(slist_reverse(&list), 0);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants_acyclic(list);

    slist_free(&list);
}

TEST(SListAlgoTest, ReverseSingleIsNoOp) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 7), 0);

    EXPECT_EQ(slist_reverse(&list), 0);
    EXPECT_EQ(to_vec_safe(&list), (std::vector<int>{7}));
    expect_invariants_acyclic(list);

    slist_free(&list);
}

TEST(SListAlgoTest, ReverseMultipleUpdatesHeadTailAndOrder) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    for (int i = 1; i <= 5; ++i) ASSERT_EQ(slist_push_back(&list, i), 0);

    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    auto old_head = list.head;
    auto old_tail = list.tail;

    EXPECT_EQ(slist_reverse(&list), 0);

    EXPECT_EQ(to_vec_safe(&list), (std::vector<int>{5,4,3,2,1}));
    // Head/tail swapped
    EXPECT_EQ(list.head, old_tail);
    EXPECT_EQ(list.tail, old_head);
    expect_invariants_acyclic(list);

    slist_free(&list);
}

TEST(SListAlgoTest, FindMiddleEmptyFails) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    int out = 0;
    EXPECT_EQ(slist_find_middle(&list, &out), -1);

    slist_free(&list);
}

TEST(SListAlgoTest, FindMiddleOddLength) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    // [1,2,3,4,5] -> middle 3
    for (int i = 1; i <= 5; ++i) ASSERT_EQ(slist_push_back(&list, i), 0);

    int out = 0;
    ASSERT_EQ(slist_find_middle(&list, &out), 0);
    EXPECT_EQ(out, 3);

    expect_invariants_acyclic(list);
    slist_free(&list);
}

TEST(SListAlgoTest, FindMiddleEvenLengthLowerMiddle) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    // [1,2,3,4] -> lower middle 2
    for (int i = 1; i <= 4; ++i) ASSERT_EQ(slist_push_back(&list, i), 0);

    int out = 0;
    ASSERT_EQ(slist_find_middle(&list, &out), 0);
    EXPECT_EQ(out, 2);

    expect_invariants_acyclic(list);
    slist_free(&list);
}

TEST(SListAlgoTest, FindMiddleDoesNotModifyList) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    for (int i = 10; i <= 60; i += 10) ASSERT_EQ(slist_push_back(&list, i), 0);

    auto before = to_vec_safe(&list);
    auto head_before = list.head;
    auto tail_before = list.tail;
    auto size_before = list.size;

    int out = 0;
    ASSERT_EQ(slist_find_middle(&list, &out), 0);

    EXPECT_EQ(to_vec_safe(&list), before);
    EXPECT_EQ(list.head, head_before);
    EXPECT_EQ(list.tail, tail_before);
    EXPECT_EQ(list.size, size_before);

    expect_invariants_acyclic(list);
    slist_free(&list);
}

TEST(SListAlgoTest, HasCycleNullArgFails) {
    EXPECT_EQ(slist_has_cycle(nullptr), -1);
}

TEST(SListAlgoTest, HasCycleEmptyIsNo) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    EXPECT_EQ(slist_has_cycle(&list), 0);

    slist_free(&list);
}

TEST(SListAlgoTest, HasCycleSingleNo) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 1), 0);

    EXPECT_EQ(slist_has_cycle(&list), 0);

    slist_free(&list);
}

TEST(SListAlgoTest, HasCycleDetectsCycleTailToHead) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    for (int i = 1; i <= 5; ++i) ASSERT_EQ(slist_push_back(&list, i), 0);

    // Create a cycle: tail->next = head
    ASSERT_NE(list.tail, nullptr);
    ASSERT_NE(list.head, nullptr);
    list.tail->next = list.head;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // IMPORTANT: break cycle before freeing to avoid infinite loop in free()
    list.tail->next = nullptr;
    slist_free(&list);
}

TEST(SListAlgoTest, HasCycleDetectsCycleToMiddle) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    for (int i = 1; i <= 6; ++i) ASSERT_EQ(slist_push_back(&list, i), 0);

    // Create a cycle: tail->next points to the 3rd node
    slist_node_t* third = list.head ? list.head->next ? list.head->next->next : nullptr : nullptr;
    ASSERT_NE(third, nullptr);
    list.tail->next = third;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // break cycle before free
    list.tail->next = nullptr;
    slist_free(&list);
}

TEST(SListAlgoTest, ReverseOnAcyclicKeepsAcyclic) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    for (int i = 0; i < 1000; ++i) ASSERT_EQ(slist_push_back(&list, i), 0);

    ASSERT_EQ(slist_has_cycle(&list), 0);
    ASSERT_EQ(slist_reverse(&list), 0);
    ASSERT_EQ(slist_has_cycle(&list), 0);

    // spot-check ends
    auto v = to_vec_safe(&list, 3);
    EXPECT_EQ(v, (std::vector<int>{999, 998, 997}));
    expect_invariants_acyclic(list);

    slist_free(&list);
}


TEST(SListCycleTest, CycleExistsEvenIfSizeIsWrongSmaller) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    build_1_to_n(&list, 5);

    // Create cycle: tail -> head
    ASSERT_NE(list.tail, nullptr);
    ASSERT_NE(list.head, nullptr);
    list.tail->next = list.head;

    // Corrupt size intentionally (simulate buggy size tracking)
    list.size = 0;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // break cycle to avoid issues in free
    list.tail->next = nullptr;
    slist_free(&list);
}

TEST(SListCycleTest, CycleExistsEvenIfSizeIsWrongBigger) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    build_1_to_n(&list, 6);

    // Create cycle: tail -> 3rd node
    slist_node_t* third = list.head ? list.head->next ? list.head->next->next : nullptr : nullptr;
    ASSERT_NE(third, nullptr);
    list.tail->next = third;

    // Corrupt size intentionally (too large)
    list.size = 1000000;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // break cycle
    list.tail->next = nullptr;
    slist_free(&list);
}

TEST(SListCycleTest, NoCycleEvenIfSizeIsWrong) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    build_1_to_n(&list, 4);

    // Corrupt size
    list.size = 1;

    EXPECT_EQ(slist_has_cycle(&list), 0);

    slist_free(&list);
}

TEST(SListCycleTest, DetectCycleWithSingleNodeSelfLoop) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 123), 0);

    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    // Create self-cycle
    list.head->next = list.head;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // break cycle
    list.head->next = nullptr;
    slist_free(&list);
}

TEST(SListCycleTest, DetectCycleInTwoNodeLoop) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);

    // head->next is node2, make node2->next point back to head
    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    list.tail->next = list.head;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // break cycle
    list.tail->next = nullptr;
    slist_free(&list);
}

TEST(SListCycleTest, DetectCycleDoesNotRelyOnTailPointer) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    build_1_to_n(&list, 5);

    // Create a cycle: tail->next = second node
    slist_node_t* second = list.head ? list.head->next : nullptr;
    ASSERT_NE(second, nullptr);
    list.tail->next = second;

    // Corrupt tail pointer (simulate tail tracking bug)
    list.tail = nullptr;

    EXPECT_EQ(slist_has_cycle(&list), 1);

    // To break cycle we need the real last node; rebuild tail by walking limited steps
    // (since we know original list had 5 nodes)
    slist_node_t* cur = list.head;
    for (int i = 0; i < 4; ++i) cur = cur->next;
    cur->next = nullptr;

    slist_free(&list);
}
