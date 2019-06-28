#ifndef __TEST_SAFE_STACK__
#define __TEST_SAFE_STACK__

#include "safe_stack.h"
#include "gtest/gtest.h"
#include <thread>
#include <chrono>

using namespace std;

TEST(stack_test, test_basic)
{
    safe_stack<int> stack;
    const uint maximum_id = 100;
  
    for (uint count = 1; count <= maximum_id; count++)
    {
        stack.push(count);
    }

    uint pop_count = maximum_id;
    while (auto res = stack.pop())
    {
        EXPECT_EQ(pop_count, *res);
        pop_count--;
    };
    EXPECT_EQ(0,pop_count);
    auto res = stack.pop();
    EXPECT_EQ(nullptr, res);
};

TEST(stack_test, test_multi_thread)
{
    safe_stack<int> stack;
    std::vector<std::thread> thread_producer;
    std::vector<std::thread> thread_consumer;
    const uint maximum_id = 10000;
    std::atomic<int> push_count(0);
    std::atomic<int> pop_count(0);
    const uint hardware = std::thread::hardware_concurrency();
    const uint virtual_thread = 6;
    int thread_num = hardware > virtual_thread ? hardware : virtual_thread;
    
    for (int i = 0; i < thread_num; i++)
    {
        thread_producer.push_back(std::thread([&stack, &maximum_id, &push_count] {
            while(push_count.load() < maximum_id)
            {
                stack.push(push_count.fetch_add(1));
                //std::this_thread::yield();
            }
        }));
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for( int i = 0; i< thread_num; i++ )
    {
        thread_consumer.push_back(std::thread([&stack, &pop_count, &maximum_id](){
            while( pop_count.load() < maximum_id )
            {
                if(stack.pop())
                {
                    pop_count.fetch_add(1);
                }
            }
        }));
    }
    std::for_each(thread_producer.begin(), thread_producer.end(), [](std::thread &thr) {
        thr.join();
    });
    std::for_each(thread_consumer.begin(), thread_consumer.end(), [](std::thread &thr) {
        thr.join();
    });
    auto res = stack.pop();
    EXPECT_EQ(nullptr, res);
};

#endif