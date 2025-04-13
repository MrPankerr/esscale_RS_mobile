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

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}