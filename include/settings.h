/**
 * @file settings.h
 * @author Lei Peng (plhitsz@outlook.com)
 * @brief A `ini` configuration file parser written in modern C++.
 * @version 3.2.0
 * @date 2024-05-08
 *
 */
#ifndef INCLUDE_SETTINGS_H_
#define INCLUDE_SETTINGS_H_

#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using Ch = char;
using StrStrMap = std::map<std::string, std::string>;

template <typename T, typename U>
struct is_decay_equiv : std::is_same<typename std::decay<T>::type, U>::type {};

// we only support std::string, int, float, double and bool ...
template <class T = void>
using enable_if_supported_type = typename std::enable_if<
    is_decay_equiv<T, std::string>::value || is_decay_equiv<T, int>::value ||
        is_decay_equiv<T, float>::value || is_decay_equiv<T, double>::value ||
        is_decay_equiv<T, bool>::value,
    bool>::type;
/**
 * @brief Return the value by the type of `T`. if the value is empty, return the
 * `default_value`
 *
 * @tparam T The expected type of the value.
 * @param value The value read from the `ini` file.
 * @param default_value The default value.
 * @return T
 */
template <typename T, enable_if_supported_type<T> = 0>
T ConvertValue(const std::string& value, const T& default_value) {
  if (value.empty()) {
    return default_value;
  }
  if constexpr (std::is_same<T, std::string>::value) {
    return value;
  } else if constexpr (std::is_same<T, int>::value) {
    std::string::size_type sz;
    return std::stoi(value, &sz);
  } else if constexpr (std::is_same<T, float>::value) {
    std::string::size_type sz;
    return std::stof(value, &sz);
  } else if constexpr (std::is_same<T, double>::value) {
    std::string::size_type sz;
    return std::stod(value, &sz);
  } else if constexpr (std::is_same<T, bool>::value) {
    return value == "true" || value == "1";
  }
  return default_value;
}

/// @brief Trim the string `s` with the locale `loc`.
template <class Str>
Str Trim(const Str& s, const std::locale& loc = std::locale()) {
  typename Str::const_iterator first = s.begin();
  typename Str::const_iterator end = s.end();
  while (first != end && std::isspace(*first, loc)) {
    ++first;
  }

  if (first == end) {
    return Str();
  }

  typename Str::const_iterator last = end;
  do {
    --last;
  } while (std::isspace(*last, loc));
  if (first != s.begin() || last + 1 != end) {
    return Str(first, last + 1);
  } else {
    return s;
  }
}

/**
 * @brief Split the string `str` with the `pattern`.
 *
 * @param str
 * @param pattern
 * @return std::vector<std::string>
 */
inline std::vector<std::string> Split(const std::string& str,
                                      const std::string& pattern) {
  std::vector<std::string> result;
  if (str.empty()) {
    return result;
  }
  size_t start = 0;
  size_t index = str.find_first_of(pattern, start);
  while (index != std::string::npos) {
    if (index != start) {
      result.push_back(str.substr(start, index - start));
    }
    start = index + 1;
    index = str.find_first_of(pattern, start);
  }
  if (!str.substr(start).empty()) {
    result.push_back(str.substr(start));
  }
  return result;
}

/**
 * @brief A class to parse `ini` setting files.
 *
 * @param `IniFullPath` The full path of the `ini` file.
 */
template <const char* IniFullPath>
class Settings {
 public:
  static Settings& GetInstance() {
    static Settings* ins = nullptr;
    if (!ins) {
      static std::once_flag flag;
      std::call_once(flag, [&]() { ins = new (std::nothrow) Settings(); });
    }
    return *ins;
  }
  // Tear down the singleton
  static void DestroyInstance() {
    Settings* ins = &GetInstance();
    delete ins;
  }
  // disable copy and move
  Settings(Settings const&) = delete;
  Settings& operator=(Settings const&) = delete;
  Settings(Settings&&) = delete;
  Settings& operator=(Settings&&) = delete;

  // ***********  interfaces ***********
  /**
   * @brief Return the full path of the `ini` file.
   *
   * @return const char*
   */
  std::string GetFullPath() const { return IniFullPath; }
  /**
   * @brief Get the value of the `key` from the `ini` file. If the `key` doesn't
   * exist, return the `default_value`.
   *
   * @tparam T The type of the value.
   * @tparam Types The type of the format string.
   * @param default_value The default value if the `key` doesn't exist.
   * @param fmt The format string to get the string of the key.
   * @param args The arguments of the format string.
   * @return T
   */
  template <typename T, typename... Types, enable_if_supported_type<T> = 0>
  T GetValue2(const T& default_value, const std::string& fmt, Types&&... args);
  /**
   * @brief Get the value of the `key` from the `ini` file. If the `key` doesn't
   * exist, return the `default_value`.
   *
   * @tparam T
   * @param key
   * @param default_value
   * @return T
   */
  template <typename T, enable_if_supported_type<T> = 0>
  T GetValue(const std::string& key, T default_value = T());
  /**
   * @brief Save/change the `value` to the `key` to the `ini` file.
   *
   * @tparam T
   * @param key The key of the value.
   * @param value The value to be saved.
   */
  template <typename T, enable_if_supported_type<T> = 0>
  void SetValue(const std::string& key, const T& value);

  friend std::ostream& operator<<(std::ostream& os, const Settings& settings) {
    for (auto& [key, value] : settings.content_tbl_) {
      os << "*" << key << " = " << value << std::endl;
    }
    return os;
  }
  void DumpFile() {
    std::ifstream ifs(IniFullPath);
    if (!ifs.is_open()) {
      std::cerr << "Failed to open file: " << IniFullPath << std::endl;
      return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
      std::cout << line << std::endl;
    }
  }

 private:
  Settings() = default;
  virtual ~Settings() = default;

  // ***********  implementation ***********
  bool LoadContentTbl();
  bool StoreContentTbl();
  void WriteIni(std::basic_ostream<char>& stream,
                const StrStrMap& ini_content_tbl);
  void ReadIni(std::basic_istream<char>& stream, StrStrMap& ini_content_tbl);
  // protect read/write
  std::mutex ini_rw_mutex_;
  StrStrMap content_tbl_;
  std::filesystem::file_time_type last_write_time_;
  // stored in memory, and write back to the ini file when SetValue is called.
};

template <const char* IniFullPath>
bool Settings<IniFullPath>::LoadContentTbl() {
  std::basic_ifstream<char> stream(IniFullPath,
                                   std::ios_base::out | std::ios_base::app);
  if (!stream) {
    return false;
  }
  stream.imbue(std::locale());
  content_tbl_.clear();
  // Read all the key-value pairs from the ini file
  ReadIni(stream, content_tbl_);
  return true;
}

template <const char* IniFullPath>
bool Settings<IniFullPath>::StoreContentTbl() {
  std::basic_ofstream<char> stream(IniFullPath);
  if (!stream) {
    return false;
  }
  stream.imbue(std::locale());
  WriteIni(stream, content_tbl_);
  last_write_time_ = std::filesystem::last_write_time(IniFullPath);
  return true;
}

template <const char* IniFullPath>
template <typename T, typename... Types, enable_if_supported_type<T>>
T Settings<IniFullPath>::GetValue2(const T& default_value,
                                   const std::string& fmt, Types&&... args) {
  std::lock_guard<std::mutex> lock(ini_rw_mutex_);
  if (!std::filesystem::exists(IniFullPath)) {
    return default_value;
  }

  auto formatString = [](const std::string& __fmt, auto&&... __args) {
    size_t args_size = snprintf(nullptr, 0, __fmt.c_str(),
                                std::forward<decltype(__args)>(__args)...) +
                       1;
    std::unique_ptr<char[]> args_buf(new char[args_size]);
    snprintf(args_buf.get(), args_size, __fmt.c_str(),
             std::forward<decltype(__args)>(__args)...);
    return std::string(args_buf.get(), args_buf.get() + args_size - 1);
  };
  std::string key = formatString(fmt, std::forward<Types>(args)...);

  // no updates, use the memory content_tbl_
  if (last_write_time_ != std::filesystem::last_write_time(IniFullPath)) {
    if (!LoadContentTbl()) {
      std::string err_msg = IniFullPath;
      err_msg += " open failed, maybe permission denied.";
      throw std::runtime_error(err_msg);
    }
    last_write_time_ = std::filesystem::last_write_time(IniFullPath);
  }
  if (content_tbl_.find(key) == content_tbl_.end()) {
    return default_value;
  }
  return ConvertValue(content_tbl_.at(key), default_value);
}

template <const char* IniFullPath>
template <typename T, enable_if_supported_type<T>>
T Settings<IniFullPath>::GetValue(const std::string& key, T default_value) {
  std::lock_guard<std::mutex> lock(ini_rw_mutex_);
  if (!std::filesystem::exists(IniFullPath)) {
    return default_value;
  }
  // no updates, use the memory content_tbl_
  if (last_write_time_ != std::filesystem::last_write_time(IniFullPath)) {
    if (!LoadContentTbl()) {
      std::string err_msg = IniFullPath;
      err_msg += " open failed, maybe permission denied.";
      throw std::runtime_error(err_msg);
    }
    last_write_time_ = std::filesystem::last_write_time(IniFullPath);
  }
  if (content_tbl_.find(key) == content_tbl_.end()) {
    return default_value;
  }
  return ConvertValue(content_tbl_.at(key), default_value);
}

template <const char* IniFullPath>
template <typename T, enable_if_supported_type<T>>
void Settings<IniFullPath>::SetValue(const std::string& key, const T& value) {
  std::lock_guard<std::mutex> lock(ini_rw_mutex_);
  if (!std::filesystem::exists(IniFullPath)) {
    std::cout << IniFullPath << " doesn't exist, create a new one."
              << std::endl;
    auto ini_parent_path = std::filesystem::path(IniFullPath).parent_path();
    if (!std::filesystem::exists(ini_parent_path)) {
      std::cout << "Create directory: " << ini_parent_path << std::endl;
      if (!std::filesystem::create_directories(ini_parent_path)) {
        // maybe permission denied
        throw std::runtime_error("Path create failed");
      }
    }
    auto create_ini_file = [](const std::string& ini_path) -> bool {
      std::ofstream file(ini_path);
      if (!file) {
        return false;
      }
      return true;
    };
    if (!create_ini_file(IniFullPath)) {
      // maybe permission denied
      throw std::runtime_error("File create failed");
    }
    std::cout << "Create regular file: " << IniFullPath << std::endl;
    last_write_time_ = std::filesystem::last_write_time(IniFullPath);
  }

  // load before write
  if (last_write_time_ != std::filesystem::last_write_time(IniFullPath)) {
    if (!LoadContentTbl()) {
      std::string err_msg = IniFullPath;
      err_msg += " open failed, maybe permission denied.";
      throw std::runtime_error(err_msg);
    }
  }

  std::string value_string;
  if constexpr (!std::is_same<typename std::decay<T>::type,
                              std::string>::value) {
    value_string = std::to_string(value);
  } else {
    value_string = value;
  }

  // insert or update
  content_tbl_.insert_or_assign(key, value_string);
  if (!StoreContentTbl()) {
    std::string err_msg = IniFullPath;
    err_msg += " write failed, maybe permission denied.";
    throw std::runtime_error(err_msg);
  }
}
/**
 * @brief Write the `ini_content_tbl` to the `stream`.
 *
 * @tparam IniFullPath
 * @param stream
 * @param ini_content_tbl The key-value tables to be written to ini files.
 */
template <const char* IniFullPath>
void Settings<IniFullPath>::WriteIni(std::basic_ostream<char>& stream,
                                     const StrStrMap& ini_content_tbl) {
  std::set<std::string> sec_name_set;
  for (auto& [combined_key, value] : ini_content_tbl) {
    if (value.empty()) {
      // always ignore empty value. it make no sense.
      continue;
    }
    auto combined_key_vec = Split(combined_key, ".");
    if (combined_key_vec.size() < 2) {
      // no section or key: invalid data
      // std::cerr << "Invalid data: " << combined_key << std::endl;
      break;
    }
    auto& section_name = combined_key_vec[0];
    if (sec_name_set.find(section_name) == sec_name_set.end()) {
      if (sec_name_set.empty()) {
        stream << Ch('[') << section_name << Ch(']') << Ch('\n');
      } else {
        stream << Ch('\n') << Ch('[') << section_name << Ch(']') << Ch('\n');
      }
      sec_name_set.insert(section_name);
    }
    std::string key;
    for (std::size_t i = 1; i < combined_key_vec.size(); i++) {
      if (!key.empty()) {
        key += ".";
      }
      key += combined_key_vec[i];
    }
    stream << key << Ch('=') << value << Ch('\n');
  }
}
/**
 * @brief Read the `stream` and store the key-value pairs in the
 * `ini_content_tbl`.
 *
 * @tparam IniFullPath
 * @param stream
 * @param ini_content_tbl
 */
template <const char* IniFullPath>
void Settings<IniFullPath>::ReadIni(std::basic_istream<char>& stream,
                                    StrStrMap& ini_content_tbl) {
  typedef std::basic_string<Ch> Str;
  const Ch semicolon = stream.widen(';');
  const Ch hash = stream.widen('#');
  const Ch lbracket = stream.widen('[');
  const Ch rbracket = stream.widen(']');

  std::string section;
  Str line;

  // For all lines
  while (stream.good()) {
    std::getline(stream, line);
    // "eof": true if an end-of-file has occurred, false otherwise.
    // "good": true if the stream error flags are all false, false otherwise.
    if (!stream.good() || stream.eof()) {
      break;
    }
    // If line is non-empty
    line = Trim(line, stream.getloc());
    if (line.empty()) {
      continue;
    }
    // Ignore comments
    if (line[0] == semicolon || line[0] == hash) {
      continue;
    }

    // section, key
    if (line[0] == lbracket) {
      typename Str::size_type end = line.find(rbracket);
      if (end == Str::npos) {
        std::cerr << "Unmatched '[' " << std::endl;
        continue;
      }
      Str key = Trim(line.substr(1, end - 1), stream.getloc());
      section = (key);
      if (ini_content_tbl.find(section) != ini_content_tbl.end()) {
        std::cerr << "Duplicated section name " << section << std::endl;
      }
    } else {
      if (section.empty()) {
        // std::cout << " unmatched section " << std::endl;
        continue;
      }
      typename Str::size_type eq_pos = line.find(Ch('='));
      if (eq_pos == Str::npos) {
        std::cerr << "Unmatched '=' " << std::endl;
        continue;
      }
      if (eq_pos == 0) {
        std::cerr << "Unmatched key " << std::endl;
        continue;
      }
      Str key = Trim(line.substr(0, eq_pos), stream.getloc());
      Str data = Trim(line.substr(eq_pos + 1, Str::npos), stream.getloc());
      std::string combined_key = section + std::string(".") + std::string(key);
      if (ini_content_tbl.find(combined_key) != ini_content_tbl.end()) {
        std::cerr << "Duplicated key name " << combined_key << std::endl;
      }
      ini_content_tbl.insert_or_assign(combined_key, (data));
    }
  }
}

#endif  // INCLUDE_SETTINGS_H_
