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

class Person : public reaction::FieldBase {
 public:
  Person(std::string name, int age, bool male)
      : m_name(field(name)), m_age(field(age)), m_male(male) {}

  std::string getName() const { return m_name.get(); }
  void setName(const std::string& name) { *m_name = name; }

  int getAge() const { return m_age.get(); }
  void setAge(int age) { *m_age = age; }

 private:
  reaction::Field<std::string> m_name;  // react对象
  reaction::Field<int> m_age;
  bool m_male;
};

TEST(BasicTest, FieldTest) {
  Person person{"lummy", 18, true};
  auto p = reaction::var(person);  // is_base_of
  auto a = reaction::var(1);
  auto ds = reaction::calc([](int aa, auto pp) { return std::to_string(aa) + pp.getName(); }, a, p);

  EXPECT_EQ(ds.get(), "1lummy");
  p.get().setName("lummy-new");
  EXPECT_EQ(ds.get(), "1lummy-new");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}