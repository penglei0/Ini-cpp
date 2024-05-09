<div align="center">

[![Ini-cpp pipelines](https://github.com/penglei0/Ini-cpp/actions/workflows/.github.yml/badge.svg)](https://github.com/penglei0/Ini-cpp/actions/workflows/.github.yml)
[![Ini-cpp CodeQL](https://github.com/penglei0/Ini-cpp/actions/workflows/codeql.github.yml/badge.svg)](https://github.com/penglei0/Ini-cpp/actions/workflows/codeql.github.yml)

</div>

---

# Introduction

This is a header only implementation of the ini file parser in modern C++.

## Build test

```bash
cmake . -B build -DCMAKE_BUILD_TYPE=Release 
cmake --build build

cd build && ctest -C Release --output-on-failure
```

## Use it in CMake project

Add the following code in your CMakeLists.txt file.

```cmake
FetchContent_Declare(
  Ini-cpp
  GIT_REPOSITORY git@github.com:penglei0/Ini-cpp.git
  GIT_TAG v1.0.0)
FetchContent_MakeAvailable(Ini-cpp)
message("Ini_cpp source directory is :" ${Ini-cpp_SOURCE_DIR})
message("Ini_cpp binary directory is :" ${Ini-cpp_BINARY_DIR})
include_directories(${Ini-cpp_SOURCE_DIR}/include)

```

## Example cpp code

```cpp
  #include <settings.h>
  constexpr const char path[] = "/etc/cfg/my_settings.ini";
  using MySettings = Settings<path>;

  auto& settings = MySettings::GetInstance();
  // string
  settings.SetValue<std::string>("string.key1", "value1");

  // float
  settings.SetValue<float>("float.key1", 1.1);

  // get operation with no default value.
  auto value = settings.GetValue<std::string>("string.key1");

  // get operation with default value.
  value = settings.GetValue<std::string>("string.key1", "default_value");

  // get operation with format args. `GetValue2` version.
  value = settings.GetValue2<std::string>("default_value", "%s%d", "string.key",1);
```
