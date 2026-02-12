#include <gtest/gtest.h>
#include <vector>
#include <random>

extern "C" {
#include "slist.h"
}

static std::vector<int> to_vec(const slist_t* list) {
    std::vector<int> out;
    for (slist_node_t* cur = list->head; cur != nullptr; cur = cur->next) {
        out.push_back(cur->value);
        // guard against accidental cycles
        if (out.size() > 200000) break;
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

TEST(SListExtraTest, PushFrontOnEmptySetsHeadAndTail) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_front(&list, 42), 0);
    EXPECT_EQ(list.size, 1u);
    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.head, list.tail);
    EXPECT_EQ(list.head->value, 42);
    EXPECT_EQ(list.head->next, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, PushBackOnEmptySetsHeadAndTail) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_back(&list, 7), 0);
    EXPECT_EQ(list.size, 1u);
    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.head, list.tail);
    EXPECT_EQ(list.tail->value, 7);
    EXPECT_EQ(list.tail->next, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, PopFrontWithNullOutValueFailsAndDoesNotModify) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);

    const auto before = to_vec(&list);
    const size_t size_before = list.size;

    EXPECT_EQ(slist_pop_front(&list, nullptr), -1);

    EXPECT_EQ(list.size, size_before);
    EXPECT_EQ(to_vec(&list), before);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, RemoveFirstWithDuplicatesRemovesOnlyFirstMatch) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    // 1,2,2,2,3
    ASSERT_EQ(slist_push_back(&list, 1), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 3), 0);

    EXPECT_EQ(slist_remove_first(&list, 2), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,2,2,3}));
    expect_invariants(list);

    EXPECT_EQ(slist_remove_first(&list, 2), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,2,3}));
    expect_invariants(list);

    EXPECT_EQ(slist_remove_first(&list, 2), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,3}));
    expect_invariants(list);

    EXPECT_EQ(slist_remove_first(&list, 2), 0);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{1,3}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, RemoveFirstUpdatesTailWhenRemovingLastNode) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    // 10,20,30
    ASSERT_EQ(slist_push_back(&list, 10), 0);
    ASSERT_EQ(slist_push_back(&list, 20), 0);
    ASSERT_EQ(slist_push_back(&list, 30), 0);

    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.tail->value, 30);

    EXPECT_EQ(slist_remove_first(&list, 30), 1);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{10,20}));
    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.tail->value, 20);
    EXPECT_EQ(list.tail->next, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, RemoveFirstOnEmptyReturnsNotFoundAndKeepsEmpty) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    EXPECT_EQ(slist_remove_first(&list, 123), 0);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, PopFrontOnSingleElementResetsTail) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_front(&list, 99), 0);
    ASSERT_EQ(list.size, 1u);

    int v = 0;
    ASSERT_EQ(slist_pop_front(&list, &v), 0);
    EXPECT_EQ(v, 99);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, PushBackAfterPopToEmptyWorks) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_back(&list, 1), 0);
    int v = 0;
    ASSERT_EQ(slist_pop_front(&list, &v), 0);
    ASSERT_EQ(v, 1);
    expect_invariants(list);

    ASSERT_EQ(slist_push_back(&list, 2), 0);
    ASSERT_EQ(slist_push_back(&list, 3), 0);
    EXPECT_EQ(to_vec(&list), (std::vector<int>{2,3}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, AlternatingPushFrontAndPushBackOrderCorrect) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    ASSERT_EQ(slist_push_front(&list, 2), 0); // [2]
    ASSERT_EQ(slist_push_back(&list, 3), 0);  // [2,3]
    ASSERT_EQ(slist_push_front(&list, 1), 0); // [1,2,3]
    ASSERT_EQ(slist_push_back(&list, 4), 0);  // [1,2,3,4]
    ASSERT_EQ(slist_push_front(&list, 0), 0); // [0,1,2,3,4]

    EXPECT_EQ(to_vec(&list), (std::vector<int>{0,1,2,3,4}));
    expect_invariants(list);

    slist_free(&list);
}

TEST(SListExtraTest, InvalidArgsReturnErrors) {
    // init
    EXPECT_EQ(slist_init(nullptr), -1);

    // free should be safe with null
    slist_free(nullptr);

    // push/pop/remove with null list
    EXPECT_EQ(slist_push_front(nullptr, 1), -1);
    EXPECT_EQ(slist_push_back(nullptr, 1), -1);

    int out = 0;
    EXPECT_EQ(slist_pop_front(nullptr, &out), -1);
    EXPECT_EQ(slist_remove_first(nullptr, 1), -1);
}

TEST(SListExtraTest, RandomizedOpsAgainstVectorModel) {
    slist_t list;
    ASSERT_EQ(slist_init(&list), 0);

    std::vector<int> model;
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> opdist(0, 4);
    std::uniform_int_distribution<int> valdist(-50, 50);

    for (int step = 0; step < 2000; ++step) {
        int op = opdist(rng);
        int val = valdist(rng);

        switch (op) {
            case 0: { // push_front
                ASSERT_EQ(slist_push_front(&list, val), 0);
                model.insert(model.begin(), val);
                break;
            }
            case 1: { // push_back
                ASSERT_EQ(slist_push_back(&list, val), 0);
                model.push_back(val);
                break;
            }
            case 2: { // pop_front
                int out = 0;
                int rc = slist_pop_front(&list, &out);
                if (model.empty()) {
                    ASSERT_EQ(rc, -1);
                } else {
                    ASSERT_EQ(rc, 0);
                    ASSERT_EQ(out, model.front());
                    model.erase(model.begin());
                }
                break;
            }
            case 3: { // remove_first(val)
                int rc = slist_remove_first(&list, val);
                auto it = std::find(model.begin(), model.end(), val);
                if (it == model.end()) {
                    ASSERT_EQ(rc, 0);
                } else {
                    ASSERT_EQ(rc, 1);
                    model.erase(it);
                }
                break;
            }
            case 4: { // no-op check invariants
                break;
            }
        }

        EXPECT_EQ(list.size, model.size());
        EXPECT_EQ(to_vec(&list), model);
        expect_invariants(list);
    }

    slist_free(&list);
}
