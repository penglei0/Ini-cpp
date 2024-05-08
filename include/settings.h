/**
 * @file settings.h
 * @author Lei Peng (plhitsz@outlook.com)
 * @brief A `ini` configuration file parser written in modern C++.
 * @version 3.2.0
 * @date 2024-05-08
 *
 */
#include <cstdarg>
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
struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type {};

// we only support std::string, int, float, double and bool ...
template <class T = void>
using enable_if_supported_type = typename std::enable_if<
    decay_equiv<T, std::string>::value || decay_equiv<T, int>::value ||
        decay_equiv<T, float>::value || decay_equiv<T, double>::value ||
        decay_equiv<T, bool>::value,
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
T ConvertValue(const std::string& value, T default_value) {
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

inline std::vector<std::string> Split(const std::string& str,
                                      const std::string& pattern) {
  std::vector<std::string> resStr;
  if ("" == str) {
    return resStr;
  }
  std::string strs = str + pattern;
  size_t pos = strs.find(pattern);
  size_t size = strs.size();

  while (pos != std::string::npos) {
    std::string x = strs.substr(0, pos);
    resStr.push_back(x);
    strs = strs.substr(pos + 1, size);
    pos = strs.find(pattern);
  }
  return resStr;
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
  // disable copy and move
  Settings(Settings const&) = delete;
  Settings& operator=(Settings const&) = delete;
  Settings(Settings&&) = delete;
  Settings& operator=(Settings&&) = delete;

  // ***********  interfaces ***********
  template <typename T, typename... Types, enable_if_supported_type<T> = 0>
  T GetValue(const std::string& fmt, Types&&... args);

  template <typename T, enable_if_supported_type<T> = 0>
  T GetValue(const std::string& key, T default_value);
  /**
   * @brief Save/change the `value` to the `key` to the `ini` file.
   *
   * @tparam T
   * @param key The key of the value.
   * @param value The value to be saved.
   */
  template <typename T, enable_if_supported_type<T> = 0>
  void SetValue(const std::string& key, T value);

 private:
  Settings() = default;
  virtual ~Settings() = default;

  // ***********  implementation ***********
  void WriteIni(std::basic_ostream<char>& stream,
                const StrStrMap& ini_content_tbl);

  void ReadIni(std::basic_istream<char>& stream, StrStrMap& ini_content_tbl);
};

template <const char* IniFullPath>
template <typename T, typename... Types, enable_if_supported_type<T>>
T Settings<IniFullPath>::GetValue(const std::string& fmt, Types&&... args) {
  T v;
  std::basic_ifstream<char> stream(IniFullPath,
                                   std::ios_base::out | std::ios_base::app);
  if (!stream) {
    std::string err_msg = IniFullPath;
    err_msg += " open failed";
    throw std::runtime_error(err_msg);
  }
  StrStrMap kv_table;
  stream.imbue(std::locale());
  // Read all the key-value pairs from the ini file
  ReadIni(stream, kv_table);

  size_t args_size =
      snprintf(nullptr, 0, fmt.c_str(), std::forward<Types>(args)...) +
      1;  // Extra space for '\0'
  std::unique_ptr<char[]> args_buf(new char[args_size]);
  snprintf(args_buf.get(), args_size, fmt.c_str(),
           std::forward<Types>(args)...);
  std::string key(args_buf.get(), args_buf.get() + args_size - 1);
  // FIXME(.): error: ‘v’ may be used uninitialized in this function
  // [-Werror=maybe-uninitialized]
  return ConvertValue(kv_table[key], v);
}

template <const char* IniFullPath>
template <typename T, enable_if_supported_type<T>>
T Settings<IniFullPath>::GetValue(const std::string& key, T default_value) {
  std::basic_ifstream<char> stream(IniFullPath,
                                   std::ios_base::out | std::ios_base::app);
  if (!stream) {
    std::string err_msg = IniFullPath;
    err_msg += " open failed";
    throw std::runtime_error(err_msg);
  }
  StrStrMap kv_table;
  stream.imbue(std::locale());

  // Read all the key-value pairs from the ini file
  ReadIni(stream, kv_table);

  return ConvertValue(kv_table[key], default_value);
}

template <const char* IniFullPath>
template <typename T, enable_if_supported_type<T>>
void Settings<IniFullPath>::SetValue(const std::string& key, T value) {
  StrStrMap kv_table;
  {
    std::basic_ifstream<char> stream(IniFullPath,
                                     std::ios_base::out | std::ios_base::app);
    if (!stream) {
      std::string err_msg = IniFullPath;
      err_msg += " open failed";
      throw std::runtime_error(err_msg);
    }
    stream.imbue(std::locale());
    // Read all the key-value pairs from the ini file
    ReadIni(stream, kv_table);
  }
  std::stringstream ss;
  std::string value_string;
  ss << value;
  ss >> value_string;
  auto it = kv_table.find(key);
  if (it != kv_table.end()) {
    it = kv_table.erase(it);
  }
  kv_table.insert(std::make_pair(key, value_string));
  // write back to the ini file
  std::basic_ofstream<char> out(IniFullPath);
  if (!out) {
    throw std::runtime_error("File open failed");
  }
  out.imbue(std::locale());
  WriteIni(out, kv_table);
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
    auto combined_key_vec = Split(combined_key, ".");
    if (combined_key_vec.size() < 2) {
      // no section or key: invalid data
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
    if (!stream.good() && !stream.eof()) {
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
      ini_content_tbl.insert(std::make_pair(combined_key, (data)));
    }
  }
}
