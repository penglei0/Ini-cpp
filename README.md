<div align="center">

[![Ini-cpp pipelines](https://github.com/penglei0/Ini-cpp/actions/workflows/.github.yml/badge.svg)](https://github.com/penglei0/Ini-cpp/actions/workflows/.github.yml)

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

## Example cpp code

```cpp
  #include "settings.h"
  constexpr const char path[] = "/etc/cfg/my_settings.ini";
  using MySettings = Settings<path>;

  auto& settings = MySettings::GetInstance();
  // string
  settings.SetValue<std::string>("string.key1", "value1");

  // int
  settings.SetValue<int>("int.key1", 1);

  // float
  settings.SetValue<float>("float.key1", 1.1);

  // get operation
  auto value = settings.GetValue<std::string>("string.key1");

```
