#include "gtest/gtest.h"

#include "test_cases/test_safe_stack.h"
#include "test_cases/test_safe_queue.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}