#include <gtest/gtest.h>

#include <iostream>

#include "settings.h"

TEST(IniSettings, ConvertValue_test) {
  EXPECT_EQ(ConvertValue<int>("", 0), 0);
  EXPECT_EQ(ConvertValue<int>("1", 0), 1);
  EXPECT_EQ(ConvertValue<int>("-1", 0), -1);

  EXPECT_EQ(ConvertValue<float>("", 0.0), 0.0);
  auto res = ConvertValue<float>("1.1", 0.0);
  EXPECT_FLOAT_EQ(res, 1.1);
  EXPECT_FLOAT_EQ(ConvertValue<float>("-1.1", 0.0), -1.1);

  EXPECT_DOUBLE_EQ(ConvertValue<double>("", 0.0), 0.0);
  EXPECT_DOUBLE_EQ(ConvertValue<double>("1.1", 0.0), 1.1);
  EXPECT_DOUBLE_EQ(ConvertValue<double>("-1.1", 0.0), -1.1);

  EXPECT_EQ(ConvertValue<bool>("", false), false);
  EXPECT_EQ(ConvertValue<bool>("1", false), true);
  EXPECT_EQ(ConvertValue<bool>("0", false), false);

  EXPECT_EQ(ConvertValue<std::string>("", "default"), "default");
  EXPECT_EQ(ConvertValue<std::string>("test", "default"), "test");

  // unsupported type
  // EXPECT_EQ(ConvertValue<std::vector<int>>("", {1, 2, 3}), {1, 2, 3});
  // EXPECT_EQ(ConvertValue<char>("", 'a'), 'a');
}

TEST(IniSettings, Trim_test) {
  std::string str = "  test  ";
  EXPECT_EQ(Trim(str), "test");

  str = "test";
  EXPECT_EQ(Trim(str), "test");

  str = "  test";
  EXPECT_EQ(Trim(str), "test");

  str = "test  ";
  EXPECT_EQ(Trim(str), "test");

  str = "  ";
  EXPECT_EQ(Trim(str), "");

  str = "";
  EXPECT_EQ(Trim(str), "");

  str = "  te st ";
  EXPECT_EQ(Trim(str), "te st");

  str = "  im  a  test  ";
  EXPECT_EQ(Trim(str), "im  a  test");

  // trim with locale
  str = "  test  ";
  EXPECT_EQ(Trim(str, std::locale("")), "test");
  std::cout << "User-preferred locale setting is "
            << std::locale("").name().c_str() << '\n';
  // Test it with Chinese characters
  str = "  你好  ";
  EXPECT_EQ(Trim(str), "你好");
}

TEST(IniSettings, Split_test) {
  std::string str = "key;value;value";
  auto vec = Split(str, ";");
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "key");
  EXPECT_EQ(vec[1], "value");
  EXPECT_EQ(vec[2], "value");

  // no trim
  str = "test, test, test";
  vec = Split(str, ",");
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "test");
  EXPECT_NE(vec[1], "test");
  EXPECT_NE(vec[2], "test");

  // split by space
  str = "test test test";
  vec = Split(str, " ");
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "test");
  EXPECT_EQ(vec[1], "test");
  EXPECT_EQ(vec[2], "test");

  // abnormal case
  str = "test";
  vec = Split(str, " ");
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], "test");

  vec = Split(str, ",");
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], "test");

  str = "test ";
  vec = Split(str, " ");
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], "test");

  vec = Split(str, ",");
  EXPECT_EQ(vec.size(), 1);
  EXPECT_NE(vec[0], "test");
  EXPECT_EQ(vec[0], "test ");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}