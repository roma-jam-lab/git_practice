#include <iostream>
#include <list>
#include <string>
#include <time.h>

#include "gtest/gtest.h"
#include "event_notifier.h"

using namespace std;

extern std::list<void*> allocatedData;
static std::string actualNotifyOrder;
static struct Exp
{
    const Event* event;
    const void* data;
    size_t length;
}expectations;

static void actual_event_handler(const Event* event, const void* data, size_t length, int id)
{
    actualNotifyOrder += std::to_string(id);
    EXPECT_EQ(expectations.event, event);
    EXPECT_EQ(expectations.data, data);
    EXPECT_EQ(expectations.length, length);
}

static void event_handler_foo0(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 0);
}
static void event_handler_foo1(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 1);
}
static void event_handler_foo2(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 2);
}
static void event_handler_foo3(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 3);
}
static void event_handler_foo4(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 4);
}
static void event_handler_foo5(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 5);
}
static void event_handler_foo6(const Event* event, const void* data, size_t length)
{
    actual_event_handler(event, data, length, 6);
}

static void setup()
{
    allocatedData.clear();
    actualNotifyOrder.clear();
}
static void teardown()
{
    ASSERT_TRUE(allocatedData.empty());
}

TEST(event_notifier_test, add_and_remove_handlers)
{
    setup();

    Event event;
    event_initialize(&event);

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_unsubscribe(&event, &event_handler_foo0));

    event_deinitialize(&event);
    teardown();
}

TEST(event_notifier_test, add_unsub_not_added)
{
    setup();

    Event event;
    event_initialize(&event);

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_FALSE(event_unsubscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_FALSE(event_unsubscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_FALSE(event_unsubscribe(&event, &event_handler_foo3));

    event_deinitialize(&event);
    ASSERT_FALSE(event_unsubscribe(&event, &event_handler_foo3));
    teardown();
}

TEST(event_notifier_test, notify_null_data)
{
    setup();

    Event event;
    event_initialize(&event);

    expectations.event = &event;
    expectations.data = nullptr;
    expectations.length = 0;

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));

    event_notify(&event, nullptr, 0);
    ASSERT_STREQ("0", actualNotifyOrder.c_str());

    event_deinitialize(&event);
    teardown();
}

TEST(event_notifier_test, check_notify_order)
{
    setup();

    Event event;
    event_initialize(&event);
    uint32_t data;

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo6));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo5));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo4));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));

    expectations.event = &event;
    expectations.data = &data;
    expectations.length = sizeof(data);
    event_notify(&event, &data, sizeof(data));
    ASSERT_STREQ("0162543", actualNotifyOrder.c_str());

    event_deinitialize(&event);
    teardown();
}

TEST(event_notifier_test, check_notify_order_rep_handlers)
{
    setup();

    Event event;
    event_initialize(&event);

    uint32_t data;

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo6));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo5));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo4));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));

    expectations.event = &event;
    expectations.data = &data;
    expectations.length = sizeof(data);
    event_notify(&event, &data, sizeof(data));
    ASSERT_STREQ("01162522143232", actualNotifyOrder.c_str());

    event_deinitialize(&event);
    teardown();
}

TEST(event_notifier_test, check_notify_order_with_unsubs)
{
    setup();

    Event event;
    event_initialize(&event);

    uint32_t data;

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_unsubscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));

    expectations.event = &event;
    expectations.data = &data;
    expectations.length = sizeof(data);
    event_notify(&event, &data, sizeof(data));
    ASSERT_STREQ("01123", actualNotifyOrder.c_str());

    event_deinitialize(&event);
    teardown();
}

TEST(event_notifier_test, multiple_notify_different_data)
{
    setup();

    Event event;
    event_initialize(&event);

    uint32_t data1, data2, data3;

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));

    expectations.event = &event;
    expectations.data = &data1;
    expectations.length = sizeof(data1);
    event_notify(&event, &data1, sizeof(data1));
    ASSERT_STREQ("01", actualNotifyOrder.c_str());

    actualNotifyOrder.clear();
    expectations.data = &data2;
    expectations.length = sizeof(data2);
    event_notify(&event, &data2, sizeof(data2));
    ASSERT_STREQ("01", actualNotifyOrder.c_str());

    actualNotifyOrder.clear();
    expectations.data = &data3;
    expectations.length = sizeof(data3);
    event_notify(&event, &data3, sizeof(data3));
    ASSERT_STREQ("01", actualNotifyOrder.c_str());

    event_deinitialize(&event);
    teardown();
}

TEST(event_notifier_test, multiple_notify_sub_and_unsub)
{
    setup();

    Event event;
    event_initialize(&event);

    uint32_t data;

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo0));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo1));

    expectations.event = &event;
    expectations.data = &data;
    expectations.length = sizeof(data);
    event_notify(&event, &data, sizeof(data));
    ASSERT_STREQ("01", actualNotifyOrder.c_str());

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo2));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_unsubscribe(&event, &event_handler_foo1));

    actualNotifyOrder.clear();
    event_notify(&event, &data, sizeof(data));
    ASSERT_STREQ("023", actualNotifyOrder.c_str());

    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_subscribe(&event, &event_handler_foo3));
    ASSERT_TRUE(event_unsubscribe(&event, &event_handler_foo2));

    actualNotifyOrder.clear();
    event_notify(&event, &data, sizeof(data));
    ASSERT_STREQ("0333", actualNotifyOrder.c_str());

    event_deinitialize(&event);
    teardown();
}
