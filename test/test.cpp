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

TEST(ReactionTest, TestReset) {
  auto a = reaction::var(1);
  auto b = reaction::var(2);
  auto ds = reaction::calc([](auto aa, auto bb) { return aa + bb; }, a, b);
  auto dds = reaction::calc([](auto aa, auto bb) { return aa + bb; }, a, b);
  dds.reset([](auto aa, auto bb) { return aa * bb; }, a, ds);
  a.value(2);
  EXPECT_EQ(dds.get(), 8);
}

TEST(ReactionTest, TestParentheses) {
  auto a = reaction::var(1);
  auto b = reaction::var(3.14);
  EXPECT_EQ(a.get(), 1);
  EXPECT_EQ(b.get(), 3.14);

  auto ds = reaction::calc([&]() { return a() + b(); });
  auto dds = reaction::calc([&]() { return std::to_string(a()) + std::to_string(ds()); });

  ASSERT_FLOAT_EQ(ds.get(), 4.14);
  EXPECT_EQ(dds.get(), "14.140000");

  a.value(2);
  ASSERT_FLOAT_EQ(ds.get(), 5.14);
  EXPECT_EQ(dds.get(), "25.140000");
}

TEST(ReactionTest, TestExpr) {
  auto a = reaction::var(1);
  auto b = reaction::var(2);
  auto c = reaction::var(3.14);
  auto ds = reaction::calc([&]() { return a() + b(); });
  auto expr_ds = reaction::expr(c + a / b - ds * 2);

  a.value(2);
  EXPECT_EQ(ds.get(), 4);
  ASSERT_FLOAT_EQ(expr_ds.get(), -3.86);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}