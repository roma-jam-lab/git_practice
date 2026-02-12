#include <gtest/gtest.h>
#include <vector>

extern "C" {
#include "slist.h"
}

static std::vector<int> to_vec(const slist_t* list) {
    std::vector<int> out;
    for (slist_node_t* cur = list->head; cur != nullptr; cur = cur->next) {
        out.push_back(cur->value);
    }
    return out;
}

static void expect_invariants(const slist_t& list) {
    // size matches traversal count
    size_t count = 0;
    slist_node_t* cur = list.head;
    slist_node_t* last = nullptr;
    while (cur) {
        last = cur;
        cur = cur->next;
        ++count;
        ASSERT_LE(count, 100000u); // safety against accidental cycles in tests
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

TEST(SListTest, InitAndFreeAreIdempotent) {
    slist_t list;
    EXPECT_EQ(slist_init(&list), 0);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    slist_free(&list);
    expect_invariants(list);

    // Call free again - must be safe
    slist_free(&list);
    expect_invariants(list);
}

TEST(SListTest, PushFrontMaintainsOrder) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_front(&list, 1), 0);
    ASSERT_EQ(slist_push_front(&list, 2), 0);
    ASSERT_EQ(slist_push_front(&list, 3), 0);

    EXPECT_EQ(to_vec(&list), (std::vector<int>{3,2,1}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, PushBackMaintainsOrder) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 3), 0);

    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,2,3}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, PopFrontFromEmptyFails) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    int v = 0;
    EXPECT_EQ(slist_pop_front(&list, &v), -1);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, PopFrontRemovesInOrder) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 10), 0);
    ASSERT_EQ(slist_push_back(&list, 20), 0);
    ASSERT_EQ(slist_push_back(&list, 30), 0);

    int v = 0;
    ASSERT_EQ(slist_pop_front(&list, &v), 0);
    EXPECT_EQ(v, 10);
    ASSERT_EQ(slist_pop_front(&list, &v), 0);
    EXPECT_EQ(v, 20);
    ASSERT_EQ(slist_pop_front(&list, &v), 0);
    EXPECT_EQ(v, 30);

    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, RemoveFirstNotFound) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);

    EXPECT_EQ(slist_remove_first(&list, 3), 0);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,2}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, RemoveFirstRemovesHead) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 3), 0);

    EXPECT_EQ(slist_remove_first(&list, 1), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{2,3}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, RemoveFirstRemovesMiddleAndTail) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 3), 0);
    ASSERT_EQ(slist_push_back(&list, 4), 0);

    EXPECT_EQ(slist_remove_first(&list, 3), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,2,4}));
    expect_invariants(list);

    EXPECT_EQ(slist_remove_first(&list, 4), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,2}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, RemoveOnlyElementResetsHeadAndTail) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);
    ASSERT_EQ(slist_push_back(&list, 7), 0);

    EXPECT_EQ(slist_remove_first(&list, 7), 1);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListTest, MixedOperationsStress) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    for (int i = 0; i < 1000; ++i) {
        ASSERT_EQ(slist_push_back(&list, i), 0);
    }
    expect_invariants(list);

    for (int i = 0; i < 500; ++i) {
        int v = -1;
        ASSERT_EQ(slist_pop_front(&list, &v), 0);
        ASSERT_EQ(v, i);
    }
    expect_invariants(list);

    // remove some values (existing and not)
    EXPECT_EQ(slist_remove_first(&list, 9999), 0);
    EXPECT_EQ(slist_remove_first(&list, 700), 1);
    EXPECT_EQ(slist_remove_first(&list, 701), 1);
    expect_invariants(list);

    slist_free(&list);
}
