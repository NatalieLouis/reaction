#include <gtest/gtest.h>

#include <string>

#include "reaction/react.h"

TEST(ReactionTest, TestCommonUse) {
  auto a = reaction::var(1);
  auto b = reaction::var(3.14);
  EXPECT_EQ(a.get(), 1);
  EXPECT_EQ(b.get(), 3.14);

  auto ds = reaction::calc([](int aa, double bb) { return aa + bb; }, a, b);
  auto dds = reaction::calc(
      [](auto aa, auto dsds) { return std::to_string(aa) + std::to_string(dsds); }, a, ds);

  ASSERT_FLOAT_EQ(ds.get(), 4.14);
  EXPECT_EQ(dds.get(), "14.140000");
  a.value(2);
  ASSERT_FLOAT_EQ(ds.get(), 5.14);
  EXPECT_EQ(dds.get(), "25.140000");
}

TEST(ReactionTest, TestMove) {
  auto a = reaction::var(1);
  auto b = reaction::var(3.14);
  auto ds = reaction::calc(
      [](int aa, double bb) { return std::to_string(aa) + std::to_string(bb); }, a, b);
  auto dds = reaction::calc([](auto aa, auto dsds) { return std::to_string(aa) + dsds; }, a, ds);

  auto dds_copy = std::move(dds);
  EXPECT_EQ(dds_copy.get(), "113.140000");
  EXPECT_FALSE(static_cast<bool>(dds));
  EXPECT_THROW(dds.get(), std::runtime_error);

  a.value(2);
  EXPECT_EQ(dds_copy.get(), "223.140000");
  EXPECT_FALSE(static_cast<bool>(dds));
}
TEST(ReactionTest, TestConst) {
  auto a = reaction::var(1);
  auto b = reaction::constVar(3.14);
  auto ds = reaction::calc([](int aa, double bb) { return aa + bb; }, a, b);
  ASSERT_FLOAT_EQ(ds.get(), 4.14);

  a.value(2);
  ASSERT_FLOAT_EQ(ds.get(), 5.14);
  // b.value(4.14);  // compile error;
}
TEST(ReactionTest, TestAction) {
  auto a = reaction::var(1);
  auto b = reaction::var(3.14);
  auto at = reaction::action(
      [](int aa, double bb) { std::cout << "a = " << aa << '\t' << "b = " << bb << '\t'; }, a,
      b);  // void类型

  a.value(2);
}

TEST(ReactionTest, TestAction2) {
  auto a = reaction::var(1);
  auto b = reaction::var(3.14);
  auto at = reaction::action(
      [](int aa, double bb) { std::cout << "a = " << aa << '\t' << "b = " << bb << '\t'; }, a, b);

  bool trigger = false;
  auto att = reaction::action(
      [&]([[maybe_unused]] auto atat) {
        trigger = true;
        std::cout << "at trigger " << std::endl;
      },
      at);

  trigger = false;

  a.value(2);
  EXPECT_TRUE(trigger);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}