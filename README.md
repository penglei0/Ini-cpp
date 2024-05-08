# Introduction

This is a header only implementation of the ini file parser in modern C++.

## Build tests

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

  // bool
  settings.SetValue<bool>("bool.key1", true);
  settings.SetValue<bool>("bool.key3", 1);

  // get operation
  auto value = settings.GetValue<std::string>("string.key1");

```
