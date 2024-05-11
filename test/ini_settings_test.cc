#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <thread>

#include "settings.h"

// it has a unique address across all translation units, and can be used as a
// non-type template argument.
constexpr const char ini_file_1[] = "/tmp/ini_settings_test_1.ini";
constexpr const char read_exist_file[] =
    "/tmp/ini_settings_test_read_exist.ini";
constexpr const char none_exists_ini_file[] =
    "/tmp/ini_settings_test_none_exists.ini";
constexpr const char my_ini_path[] = "/tmp/my_settings.ini";

using MySettings = Settings<my_ini_path>;
using IniSettings1 = Settings<ini_file_1>;
using ExistSettings = Settings<read_exist_file>;

TEST(IniSettings, read_none_exists_file) {
  using IniSettings2 = Settings<none_exists_ini_file>;
  // check exist
  if (std::filesystem::exists(none_exists_ini_file)) {
    std::filesystem::remove(none_exists_ini_file);
  }
  // read
  EXPECT_EQ(
      IniSettings2::GetInstance().GetValue<std::string>("main.key1", "default"),
      "default");
  std::filesystem::remove(none_exists_ini_file);
}

TEST(IniSettings, write_none_exists_file) {
  using IniSettings2 = Settings<none_exists_ini_file>;
  // check exist
  if (std::filesystem::exists(none_exists_ini_file)) {
    std::filesystem::remove(none_exists_ini_file);
  }
  // invalid write since no section name
  IniSettings2::GetInstance().SetValue<std::string>("key1", "value1");
  EXPECT_TRUE(std::filesystem::exists(none_exists_ini_file));
  EXPECT_EQ(std::filesystem::file_size(none_exists_ini_file), 0);

  IniSettings2::GetInstance().SetValue<std::string>("main.key1", "value1");
  EXPECT_EQ(
      IniSettings2::GetInstance().GetValue<std::string>("main.key1", "default"),
      "value1");
  std::filesystem::remove(none_exists_ini_file);
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

  settings.DumpFile();

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

  // multiple args
  res =
      settings.GetValue2<std::string>("default_value", "%s%d", "string.key", 1);
  EXPECT_EQ(res, "value1");
}

TEST(IniSettings, multithread_rw_test) {
  std::filesystem::remove(IniSettings1::GetInstance().GetFullPath());
  auto& settings = IniSettings1::GetInstance();
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "value1"), "value1");
  settings.SetValue<std::string>("string.key1", "value2");

  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "value1"), "value2");
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "value1"), "value2");
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "value1"), "value2");
  std::atomic<bool> is_ready = {false};
  std::thread read_thread = std::thread([&settings, &is_ready]() {
    while (!is_ready.load()) {
      std::this_thread::yield();
    }
    for (int i = 0; i < 3000; ++i) {
      auto res = settings.GetValue<std::string>("string.key1", "value1");
      EXPECT_TRUE(res == "value2" || res == "value3");
    }
    std::cout << "read_thread done" << std::endl;
  });

  std::thread write_thread = std::thread([&settings, &is_ready]() {
    is_ready.store(true);
    for (int i = 0; i < 3000; ++i) {
      settings.SetValue<std::string>("string.key1", "value3");
    }
    std::cout << "write_thread done" << std::endl;
    IniSettings1::GetInstance().DumpFile();
    std::cout << "========================== " << std::endl;
  });

  read_thread.join();
  write_thread.join();
  std::cout << "start modify" << std::endl;
  // add space to `ini_file_1` by executing bash cmd echo " " > ini_file_1
  std::string cmd = "echo \" \" >> ";
  cmd += ini_file_1;

  std::ignore = system(cmd.c_str());

  auto res = settings.GetValue<std::string>("string.key1", "value1");
  EXPECT_TRUE(res == "value3");

  std::filesystem::remove(IniSettings1::GetInstance().GetFullPath());
  // it will not use the cache.
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "delete"), "delete");
}
TEST(IniSettings, multithread_www_test) {
  auto& settings = MySettings::GetInstance();
  settings.GetValue<std::string>("string.key1", "value1");
  std::vector<std::thread> threads;
  std::atomic<bool> is_ready = {false};
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&settings, &is_ready, i]() {
      if (i == 9) {
        is_ready.store(true);
      }
      while (!is_ready.load()) {
        std::this_thread::yield();
      }
      std::string thread_value_str = "value";
      thread_value_str += std::to_string(i + 10);
      for (int c = 0; c < 3000; ++c) {
        settings.SetValue<std::string>("string.key1", thread_value_str);
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  auto res = settings.GetValue<std::string>("string.key1", "value1");
  EXPECT_TRUE(res == "value10" || res == "value11" || res == "value12" ||
              res == "value13" || res == "value14" || res == "value15" ||
              res == "value16" || res == "value17" || res == "value18" ||
              res == "value19");
  MySettings::GetInstance().DumpFile();
}
/// @brief Read a non-exist key, it will return the default value; and it's
/// default value should not be stored.
/// @param
/// @param
TEST(IniSettings, read_default_value_should_not_be_stored) {
  std::filesystem::remove(IniSettings1::GetInstance().GetFullPath());
  std::ofstream file(IniSettings1::GetInstance().GetFullPath());
  if (!file) {
    EXPECT_FALSE(true);
  }
  auto& settings = IniSettings1::GetInstance();
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "default_value1"),
            "default_value1");
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "default_value2"),
            "default_value2");
  // set
  settings.SetValue<std::string>("string.key1", "value1");
  EXPECT_EQ(settings.GetValue<std::string>("string.key1", "value1"), "value1");
  // std::filesystem::remove(IniSettings1::GetInstance().GetFullPath());
}

TEST(IniSettings, abnormal_write_test) {
  // no section name
  EXPECT_EQ(MySettings::GetInstance().GetValue<std::string>("key1", "default"),
            "default");
  // invalid write
  MySettings::GetInstance().SetValue<std::string>("key1", "value1");
  MySettings::GetInstance().DumpFile();
}

const char my_ini_content[] = R"(
[bool]
key1=1
key2=0
#key3=0

[float]
key1=1.100000
key2=2.200000
;key3=3.300000
[int]
key1=1
key2=2

[string]
key1=value11
key2=value22

#
#
)";
TEST(IniSettings, read_exist_file) {
  std::filesystem::remove(ExistSettings::GetInstance().GetFullPath());
  // write content to file `my_ini_path`
  std::ofstream file(ExistSettings::GetInstance().GetFullPath());
  if (!file) {
    EXPECT_FALSE(true);
  }
  file << my_ini_content;
  file.close();

  EXPECT_EQ(ExistSettings::GetInstance().GetValue<std::string>("string.key1",
                                                               "default"),
            "value11");
  EXPECT_EQ(ExistSettings::GetInstance().GetValue<std::string>("string.key2"),
            "value22");
  EXPECT_EQ(ExistSettings::GetInstance().GetValue<int>("int.key1"), 1);
  EXPECT_EQ(ExistSettings::GetInstance().GetValue<int>("int.key2"), 2);
  EXPECT_FLOAT_EQ(ExistSettings::GetInstance().GetValue<float>("float.key1"),
                  1.1);
  EXPECT_FLOAT_EQ(ExistSettings::GetInstance().GetValue<float>("float.key2"),
                  2.2);
  EXPECT_FLOAT_EQ(
      ExistSettings::GetInstance().GetValue<float>("float.key3", 3.10), 3.10);
  EXPECT_EQ(ExistSettings::GetInstance().GetValue<bool>("bool.key1"), true);
  EXPECT_EQ(ExistSettings::GetInstance().GetValue<bool>("bool.key2"), false);
  EXPECT_EQ(ExistSettings::GetInstance().GetValue<bool>("bool.key3", true),
            true);
  std::filesystem::remove(ExistSettings::GetInstance().GetFullPath());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}