#include "../include/utils.h"
#include <gtest/gtest.h>

TEST(UtilsTest, IsEvenFunction) {
    EXPECT_TRUE(isEven(2));
    EXPECT_FALSE(isEven(3));
}

TEST(UtilsTest, HelloOutput) {
    testing::internal::CaptureStdout();
    printHello();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "Hello from backend!\n");
}