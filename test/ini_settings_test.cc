#include <gtest/gtest.h>

#include "settings.h"

// it has a unique address across all translation units, and can be used as a
// non-type template argument.
constexpr const char ini_file_1[] = "/tmp/ini_settings_test_1.ini";
constexpr const char ini_file_2[] = "/tmp/ini_settings_test_2.ini";

constexpr const char path[] = "/tmp/my_settings.ini";
using MySettings = Settings<path>;

using IniSettings1 = Settings<ini_file_1>;
using IniSettings2 = Settings<ini_file_2>;

void DumpFileContent(const std::string& file) {
  std::ifstream ifs(file);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open file: " << file << std::endl;
    return;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    std::cout << line << std::endl;
  }
}

TEST(IniSettings, write_read_test) {
  auto& settings = IniSettings1::GetInstance();
  // string
  settings.SetValue<std::string>("string.key1", "value1");
  settings.SetValue<std::string>("string.key2", "value2");
  settings.SetValue<std::string>("string.key3", "value3");

  // int
  settings.SetValue<int>("int.key1", 1);
  settings.SetValue<int>("int.key2", 2);
  settings.SetValue<int>("int.key3", 3);

  // float
  settings.SetValue<float>("float.key1", 1.1);
  settings.SetValue<float>("float.key2", 2.2);
  settings.SetValue<float>("float.key3", 3.3);

  // bool
  settings.SetValue<bool>("bool.key1", true);
  settings.SetValue<bool>("bool.key2", false);
  settings.SetValue<bool>("bool.key3", 1);

  DumpFileContent(ini_file_1);

  // read it back
  // string
  EXPECT_EQ(settings.GetValue<std::string>("string.key1"), "value1");
  EXPECT_EQ(settings.GetValue<std::string>("string.key2"), "value2");
  EXPECT_EQ(settings.GetValue<std::string>("string.key3"), "value3");
  // not exist
  EXPECT_EQ(settings.GetValue<std::string>("string.key4"), "");

  // int
  EXPECT_EQ(settings.GetValue<int>("int.key1"), 1);
  EXPECT_EQ(settings.GetValue<int>("int.key2"), 2);
  EXPECT_EQ(settings.GetValue<int>("int.key3"), 3);
  // not exist
  EXPECT_EQ(settings.GetValue<int>("int.key4"), 0);

  // float
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key1"), 1.1);
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key2"), 2.2);
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key3"), 3.3);
  // not exist
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key4"), 0.0);

  // bool
  EXPECT_EQ(settings.GetValue<bool>("bool.key1"), true);
  EXPECT_EQ(settings.GetValue<bool>("bool.key2"), false);
  EXPECT_EQ(settings.GetValue<bool>("bool.key3"), true);

  // formatting read string.key%d
  for (int i = 1; i <= 3; ++i) {
    std::string results = "value";
    results += std::to_string(i);
    EXPECT_EQ(settings.GetValue2<std::string>("default_str", "string.key%d", i),
              results);
  }
  // calling none-format version
  auto res = settings.GetValue<std::string>("string.key10", "default_value");
  EXPECT_EQ(res, "default_value");
  res = settings.GetValue2<std::string>("default_str", "%s10", "string.key");
}

TEST(IniSettings, abnormal_write_test) {
  // no section name
  EXPECT_EQ(MySettings::GetInstance().GetValue<std::string>("key1", "default"),
            "default");
  // invalid write
  MySettings::GetInstance().SetValue<std::string>("key1", "value1");
  DumpFileContent(MySettings::GetInstance().GetFullPath());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}