#include "gtest/gtest.h"
TEST(BasicTest, CalcTest)
{
    auto a = 1;
    auto b = 2;
    EXPECT_EQ(a + b, 3);
}
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}