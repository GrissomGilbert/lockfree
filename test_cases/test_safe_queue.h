#ifndef __TEST_SAFE_QUEUE__
#define __TEST_SAFE_QUEUE__

#include "gtest/gtest.h"
#include "safe_queue.h"

#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

TEST(queue_test, test_basic)
{
    safe_queue<int> queue;
    queue.enqueue(1);
    int expect = 1;
    auto res = queue.find_if([&expect](std::shared_ptr<int> val) -> bool {
        return val && expect == *val ? true : false;
    });
    ASSERT_NE(nullptr, res);
    EXPECT_EQ(1, *res);
    res = queue.dequeue();
    ASSERT_NE(nullptr, res);
    EXPECT_EQ(1, *res);
    res = queue.dequeue();
    EXPECT_EQ(nullptr, res);
};

TEST(queue_test, test_muti_value)
{
    safe_queue<int> queue;
    uint numbers = 100;
    std::thread producer([&queue, &numbers]() {
        for (int i = 0; i < numbers; i++)
        {
            queue.enqueue(i + 1);
        }
    });
    producer.join();
    int expect = 30;
    auto res = queue.find_if([&expect](std::shared_ptr<int> val) -> bool {
        return val && expect == *val ? true : false;
    });
    ASSERT_NE(nullptr, res);
    EXPECT_EQ(expect, *res);

    std::promise<int> pro_count;
    auto fut_count = pro_count.get_future();
    std::thread consumer([&queue, &pro_count]() {
        uint count = 0;
        while (queue.dequeue())
        {
            count++;
        };
        pro_count.set_value(count);
    });
    consumer.join();
    EXPECT_EQ(numbers, fut_count.get());
    res = queue.dequeue();
    EXPECT_EQ(nullptr, res);
}

TEST(queue_test, test_mult_thread)
{
    safe_queue<int> queue;
    const uint max_numbers = 100000;
    std::atomic<uint> producer_count(0);
    std::atomic<uint> consumer_count(max_numbers);
    uint max_thread = 6;
    std::vector<std::thread> vec_producer;
    std::vector<std::thread> vec_consumer;

    for (int i = 0; i < max_thread; i++)
    {
        vec_producer.push_back(std::thread([&queue, &producer_count, &max_numbers]() {
            do
            {
                queue.enqueue(producer_count.fetch_add(1));
                //std::this_thread::yield();
            } while (producer_count.load() < max_numbers);
        }));
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (int i = 0; i < max_thread; i++)
    {
        vec_consumer.push_back(std::thread([&queue, &consumer_count]() {
            do
            {
                if (queue.dequeue())
                    consumer_count.fetch_sub(1);
                //std::this_thread::yield();
            } while (consumer_count.load());
        }));
    }

    std::for_each(vec_producer.begin(), vec_producer.end(), [](std::thread &thr) {
        thr.join();
    });
    std::for_each(vec_consumer.begin(), vec_consumer.end(), [](std::thread &thr) {
        thr.join();
    });

    EXPECT_EQ(max_numbers, producer_count);
    EXPECT_EQ(0, queue.get_count());
}

#endif