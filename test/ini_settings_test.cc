#include <gtest/gtest.h>

#include "settings.h"

// it has a unique address across all translation units, and can be used as a
// non-type template argument.
constexpr const char ini_file_1[] = "/tmp/ini_settings_test_1.ini";
constexpr const char ini_file_2[] = "/tmp/ini_settings_test_2.ini";

constexpr const char path[] = "/etc/cfg/my_settings.ini";
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

  // int
  EXPECT_EQ(settings.GetValue<int>("int.key1"), 1);
  EXPECT_EQ(settings.GetValue<int>("int.key2"), 2);
  EXPECT_EQ(settings.GetValue<int>("int.key3"), 3);

  // float
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key1"), 1.1);
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key2"), 2.2);
  EXPECT_FLOAT_EQ(settings.GetValue<float>("float.key3"), 3.3);

  // bool
  EXPECT_EQ(settings.GetValue<bool>("bool.key1"), true);
  EXPECT_EQ(settings.GetValue<bool>("bool.key2"), false);
  EXPECT_EQ(settings.GetValue<bool>("bool.key3"), true);
}

TEST(IniSettings, abnormal_write_test) { EXPECT_FALSE(false); }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}