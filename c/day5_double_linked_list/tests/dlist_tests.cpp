// tests/dlist_test.cpp
// GoogleTest suite for day5_dlist (doubly linked list)

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <random>

extern "C" {
#include "dlist.h"
}

static std::vector<int> to_vec_forward(const dlist_t* list, size_t limit = 200000) {
    std::vector<int> out;
    const dlist_node_t* cur = list->head;
    while (cur && out.size() < limit) {
        out.push_back(cur->value);
        cur = cur->next;
    }
    return out;
}

static std::vector<int> to_vec_backward(const dlist_t* list, size_t limit = 200000) {
    std::vector<int> out;
    const dlist_node_t* cur = list->tail;
    while (cur && out.size() < limit) {
        out.push_back(cur->value);
        cur = cur->prev;
    }
    return out;
}

// Strict invariants for acyclic doubly linked list
static void expect_invariants(const dlist_t& list) {
    // Empty invariants
    if (list.size == 0) {
        EXPECT_EQ(list.head, nullptr);
        EXPECT_EQ(list.tail, nullptr);
        return;
    }

    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);

    // Head/tail boundary invariants
    EXPECT_EQ(list.head->prev, nullptr);
    EXPECT_EQ(list.tail->next, nullptr);

    // Forward traversal: next/prev consistency and count
    size_t count_f = 0;
    const dlist_node_t* cur = list.head;
    const dlist_node_t* last = nullptr;

    while (cur) {
        last = cur;

        if (cur->next) {
            EXPECT_EQ(cur->next->prev, cur);
        } else {
            // if no next, this must be the tail
            EXPECT_EQ(cur, list.tail);
        }

        if (cur->prev) {
            EXPECT_EQ(cur->prev->next, cur);
        } else {
            // if no prev, this must be the head
            EXPECT_EQ(cur, list.head);
        }

        cur = cur->next;
        ++count_f;
        ASSERT_LE(count_f, 200000u); // guard vs cycles
    }

    EXPECT_EQ(last, list.tail);
    EXPECT_EQ(count_f, list.size);

    // Backward traversal: consistency and count
    size_t count_b = 0;
    cur = list.tail;
    const dlist_node_t* first = nullptr;

    while (cur) {
        first = cur;

        if (cur->prev) {
            EXPECT_EQ(cur->prev->next, cur);
        } else {
            EXPECT_EQ(cur, list.head);
        }

        if (cur->next) {
            EXPECT_EQ(cur->next->prev, cur);
        } else {
            EXPECT_EQ(cur, list.tail);
        }

        cur = cur->prev;
        ++count_b;
        ASSERT_LE(count_b, 200000u);
    }

    EXPECT_EQ(first, list.head);
    EXPECT_EQ(count_b, list.size);

    // Forward == reverse(reverse(backward))
    auto f = to_vec_forward(&list);
    auto b = to_vec_backward(&list);
    std::reverse(b.begin(), b.end());
    EXPECT_EQ(f, b);
}

/* =========================
   Basic lifecycle tests
   ========================= */

TEST(DListTest, InitEmpty) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);
    dlist_free(&list);
}

TEST(DListTest, FreeIsIdempotentAndResetsState) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    for (int i = 0; i < 10; ++i) ASSERT_EQ(dlist_push_back(&list, i), 0);
    ASSERT_EQ(list.size, 10u);

    dlist_free(&list);
    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    dlist_free(&list);
    expect_invariants(list);
}

TEST(DListTest, InvalidArgs) {
    EXPECT_EQ(dlist_init(nullptr), -1);
    dlist_free(nullptr);
    EXPECT_EQ(dlist_push_front(nullptr, 1), -1);
    EXPECT_EQ(dlist_push_back(nullptr, 1), -1);

    int out = 0;
    EXPECT_EQ(dlist_pop_front(nullptr, &out), -1);
    EXPECT_EQ(dlist_pop_back(nullptr, &out), -1);
    EXPECT_EQ(dlist_remove_first(nullptr, 1), -1);
}

/* =========================
   Push operations
   ========================= */

TEST(DListTest, PushFrontOnEmptySetsHeadAndTail) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_front(&list, 42), 0);
    EXPECT_EQ(list.size, 1u);
    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.head, list.tail);
    EXPECT_EQ(list.head->value, 42);
    EXPECT_EQ(list.head->prev, nullptr);
    EXPECT_EQ(list.head->next, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PushBackOnEmptySetsHeadAndTail) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 7), 0);
    EXPECT_EQ(list.size, 1u);
    ASSERT_NE(list.head, nullptr);
    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.head, list.tail);
    EXPECT_EQ(list.tail->value, 7);
    EXPECT_EQ(list.tail->prev, nullptr);
    EXPECT_EQ(list.tail->next, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PushFrontMaintainsOrder) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_front(&list, 1), 0);
    ASSERT_EQ(dlist_push_front(&list, 2), 0);
    ASSERT_EQ(dlist_push_front(&list, 3), 0);

    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{3,2,1}));
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PushBackMaintainsOrder) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 3), 0);

    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{1,2,3}));
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, MixedPushFrontBackOrder) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_front(&list, 2), 0); // [2]
    ASSERT_EQ(dlist_push_back(&list, 3), 0);  // [2,3]
    ASSERT_EQ(dlist_push_front(&list, 1), 0); // [1,2,3]
    ASSERT_EQ(dlist_push_back(&list, 4), 0);  // [1,2,3,4]
    ASSERT_EQ(dlist_push_front(&list, 0), 0); // [0,1,2,3,4]

    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{0,1,2,3,4}));
    expect_invariants(list);

    dlist_free(&list);
}

/* =========================
   Pop operations
   ========================= */

TEST(DListTest, PopFrontEmptyFails) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    int out = 0;
    EXPECT_EQ(dlist_pop_front(&list, &out), -1);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PopBackEmptyFails) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    int out = 0;
    EXPECT_EQ(dlist_pop_back(&list, &out), -1);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PopFrontNullOutFailsAndDoesNotModify) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);
    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);

    auto before = to_vec_forward(&list);
    auto size_before = list.size;

    EXPECT_EQ(dlist_pop_front(&list, nullptr), -1);
    EXPECT_EQ(list.size, size_before);
    EXPECT_EQ(to_vec_forward(&list), before);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PopBackNullOutFailsAndDoesNotModify) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);
    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);

    auto before = to_vec_forward(&list);
    auto size_before = list.size;

    EXPECT_EQ(dlist_pop_back(&list, nullptr), -1);
    EXPECT_EQ(list.size, size_before);
    EXPECT_EQ(to_vec_forward(&list), before);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PopFrontRemovesInOrderAndResetsOnLast) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);
    ASSERT_EQ(dlist_push_back(&list, 10), 0);
    ASSERT_EQ(dlist_push_back(&list, 20), 0);
    ASSERT_EQ(dlist_push_back(&list, 30), 0);

    int out = 0;
    ASSERT_EQ(dlist_pop_front(&list, &out), 0);
    EXPECT_EQ(out, 10);
    expect_invariants(list);

    ASSERT_EQ(dlist_pop_front(&list, &out), 0);
    EXPECT_EQ(out, 20);
    expect_invariants(list);

    ASSERT_EQ(dlist_pop_front(&list, &out), 0);
    EXPECT_EQ(out, 30);

    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, PopBackRemovesInOrderAndResetsOnLast) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);
    ASSERT_EQ(dlist_push_back(&list, 10), 0);
    ASSERT_EQ(dlist_push_back(&list, 20), 0);
    ASSERT_EQ(dlist_push_back(&list, 30), 0);

    int out = 0;
    ASSERT_EQ(dlist_pop_back(&list, &out), 0);
    EXPECT_EQ(out, 30);
    expect_invariants(list);

    ASSERT_EQ(dlist_pop_back(&list, &out), 0);
    EXPECT_EQ(out, 20);
    expect_invariants(list);

    ASSERT_EQ(dlist_pop_back(&list, &out), 0);
    EXPECT_EQ(out, 10);

    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

/* =========================
   Remove operations
   ========================= */

TEST(DListTest, RemoveFirstOnEmptyReturnsNotFound) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    EXPECT_EQ(dlist_remove_first(&list, 123), 0);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, RemoveFirstNotFoundDoesNotChange) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 3), 0);

    auto before = to_vec_forward(&list);
    auto size_before = list.size;
    auto head_before = list.head;
    auto tail_before = list.tail;

    EXPECT_EQ(dlist_remove_first(&list, 999), 0);
    EXPECT_EQ(list.size, size_before);
    EXPECT_EQ(list.head, head_before);
    EXPECT_EQ(list.tail, tail_before);
    EXPECT_EQ(to_vec_forward(&list), before);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, RemoveFirstRemovesHead) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 3), 0);

    EXPECT_EQ(dlist_remove_first(&list, 1), 1);
    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{2,3}));
    ASSERT_NE(list.head, nullptr);
    EXPECT_EQ(list.head->prev, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, RemoveFirstRemovesTail) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 3), 0);

    EXPECT_EQ(dlist_remove_first(&list, 3), 1);
    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{1,2}));
    ASSERT_NE(list.tail, nullptr);
    EXPECT_EQ(list.tail->next, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, RemoveFirstRemovesMiddle) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 3), 0);
    ASSERT_EQ(dlist_push_back(&list, 4), 0);

    EXPECT_EQ(dlist_remove_first(&list, 3), 1);
    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{1,2,4}));
    expect_invariants(list);

    dlist_free(&list);
}

TEST(DListTest, RemoveFirstWithDuplicatesRemovesOnlyFirst) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    // 1,2,2,2,3
    ASSERT_EQ(dlist_push_back(&list, 1), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 2), 0);
    ASSERT_EQ(dlist_push_back(&list, 3), 0);

    EXPECT_EQ(dlist_remove_first(&list, 2), 1);
    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{1,2,2,3}));
    expect_invariants(list);

    EXPECT_EQ(dlist_remove_first(&list, 2), 1);
    EXPECT_EQ(to_vec_forward(&list), (std::vector<int>{1,2,3}));
    expect_invariants(list);

    slist_free: ; // (intentionally no-op label; ignore)
    dlist_free(&list);
}

TEST(DListTest, RemoveOnlyElementResetsHeadAndTail) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    ASSERT_EQ(dlist_push_back(&list, 7), 0);
    EXPECT_EQ(dlist_remove_first(&list, 7), 1);

    EXPECT_EQ(list.size, 0u);
    EXPECT_EQ(list.head, nullptr);
    EXPECT_EQ(list.tail, nullptr);
    expect_invariants(list);

    dlist_free(&list);
}

/* =========================
   Stress / model-based tests
   ========================= */

TEST(DListTest, RandomizedOpsAgainstVectorModel) {
    dlist_t list;
    ASSERT_EQ(dlist_init(&list), 0);

    std::vector<int> model;
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> opdist(0, 5);
    std::uniform_int_distribution<int> valdist(-50, 50);

    for (int step = 0; step < 4000; ++step) {
        int op = opdist(rng);
        int val = valdist(rng);

        switch (op) {
            case 0: { // push_front
                ASSERT_EQ(dlist_push_front(&list, val), 0);
                model.insert(model.begin(), val);
                break;
            }
            case 1: { // push_back
                ASSERT_EQ(dlist_push_back(&list, val), 0);
                model.push_back(val);
                break;
            }
            case 2: { // pop_front
                int out = 0;
                int rc = dlist_pop_front(&list, &out);
                if (model.empty()) {
                    ASSERT_EQ(rc, -1);
                } else {
                    ASSERT_EQ(rc, 0);
                    ASSERT_EQ(out, model.front());
                    model.erase(model.begin());
                }
                break;
            }
            case 3: { // pop_back
                int out = 0;
                int rc = dlist_pop_back(&list, &out);
                if (model.empty()) {
                    ASSERT_EQ(rc, -1);
                } else {
                    ASSERT_EQ(rc, 0);
                    ASSERT_EQ(out, model.back());
                    model.pop_back();
                }
                break;
            }
            case 4: { // remove_first(val)
                int rc = dlist_remove_first(&list, val);
                auto it = std::find(model.begin(), model.end(), val);
                if (it == model.end()) {
                    ASSERT_EQ(rc, 0);
                } else {
                    ASSERT_EQ(rc, 1);
                    model.erase(it);
                }
                break;
            }
            case 5: { // no-op check
                break;
            }
        }

        EXPECT_EQ(list.size, model.size());
        EXPECT_EQ(to_vec_forward(&list), model);

        auto back = to_vec_backward(&list);
        std::reverse(back.begin(), back.end());
        EXPECT_EQ(back, model);

        expect_invariants(list);
    }

    dlist_free(&list);
}
