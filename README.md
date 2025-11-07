# Roaring Bitmap Compare

## 项目说明

本项目用于对比 Roaring Bitmap v1 (0.3.1) 和 v2 (4.4.2) 的 `getSizeInBytes()` 方法返回值。

**此项目的所有文件（源码、CMakeLists.txt 以及文档等）均由 AI 大模型 Claude-4.5-Sonnet 生成。**

## 项目结构

```
/root/test/
├── CMakeLists.txt          # 主构建文件
├── main.cpp                # 主程序，对比两个版本
├── v1/                     # Roaring Bitmap v1 源码
└── v2/                     # Roaring Bitmap v2 源码
```

## 技术方案

由于 v1 和 v2 的头文件名称相同（`roaring/roaring64map.hh`），直接在同一个源文件中包含会产生冲突。本项目采用以下方案：

1. **分离编译**：将 v1 和 v2 的测试代码分别编译为独立的静态库
2. **C 接口隔离**：通过 `extern "C"` 导出测试函数，避免 C++ 符号冲突
3. **主程序调用**：main.cpp 通过 C 接口调用两个版本的测试函数

### CMakeLists.txt 关键点

```cmake
# 1. 为 v1 设置库名称并构建
set(ROARING_LIB_NAME "roaring_v1" CACHE STRING "v1 library name" FORCE)
add_subdirectory(v1 ${CMAKE_BINARY_DIR}/v1_build)

# 2. 动态修补 v2 的 CMakeLists.txt，将目标名从 "roaring" 改为 "roaring_v2"
# 这样可以避免与 v1 的目标名冲突
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/v2/src/CMakeLists.txt" V2_CMAKE_CONTENT)
string(REPLACE "add_library(roaring " "add_library(roaring_v2 " V2_CMAKE_CONTENT "${V2_CMAKE_CONTENT}")
# ... 更多替换 ...
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/v2_src_CMakeLists.txt" "${V2_CMAKE_CONTENT}")

# 3. 备份原始文件并使用修补版本
add_subdirectory(v2 ${CMAKE_BINARY_DIR}/v2_build)

# 4. 创建 v1 测试库（使用 v1 的头文件）
add_library(test_v1_lib STATIC test_v1.cpp)
target_include_directories(test_v1_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/v1/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v1/cpp
)
target_link_libraries(test_v1_lib PRIVATE roaring_v1)

# 5. 创建 v2 测试库（使用 v2 的头文件）
add_library(test_v2_lib STATIC test_v2.cpp)
target_include_directories(test_v2_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/v2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v2/cpp
)
target_link_libraries(test_v2_lib PRIVATE roaring_v2)

# 6. 主程序链接两个测试库
add_executable(main main.cpp)
target_link_libraries(main PRIVATE test_v1_lib test_v2_lib)
```

**目标名称冲突解决方案**：
- v1 使用变量 `ROARING_LIB_NAME`，我们设置为 `roaring_v1`
- v2 硬编码了目标名 `roaring`，我们在配置时动态修补其 CMakeLists.txt，将所有 `roaring` 替换为 `roaring_v2`
- 原始的 v2/src/CMakeLists.txt 会被备份为 CMakeLists.txt.original
- 运行 `./clean.sh` 可以恢复原始文件

### main.cpp 关键点

```cpp
// 声明外部 C 函数
extern "C" {
    size_t test_roaring_v1(const uint64_t* values, size_t count, bool portable);
    size_t test_roaring_v2(const uint64_t* values, size_t count, bool portable);
}

// 对比函数
void compare_size(const std::vector<uint64_t>& values) {
    size_t v1_size = test_roaring_v1(values.data(), values.size(), true);
    size_t v2_size = test_roaring_v2(values.data(), values.size(), true);

    std::cout << "V1: " << v1_size << " bytes" << std::endl;
    std::cout << "V2: " << v2_size << " bytes" << std::endl;
    std::cout << "Match: " << (v1_size == v2_size ? "YES" : "NO") << std::endl;
}
```

## 构建步骤

### 1. 创建构建目录

```bash
cd /root/test
mkdir build
cd build
```

### 2. 配置项目

```bash
cmake ..
```

### 3. 编译

```bash
make -j$(nproc)
```

或者使用 cmake 构建命令：

```bash
cmake --build . -j$(nproc)
```

### 4. 运行

```bash
./main
```

## 测试场景

程序会测试以下场景：

1. **空位图**：测试空 bitmap 的序列化大小
2. **单个元素**：测试只有一个元素的情况
3. **小密集范围**：0-99 的连续整数
4. **大密集范围**：0-9999 的连续整数
5. **稀疏数据**：间隔 1000 的数据
6. **极稀疏数据**：间隔 1M 的数据
7. **混合模式**：密集 + 稀疏的组合
8. **大数值**：使用高 32 位的数据
9. **多桶**：跨越多个 32 位桶的数据
10. **非便携格式**：测试非便携序列化格式
11. **伪随机模式**：模拟随机数据
12. **2的幂次**：特殊的稀疏模式

## 预期结果

### 可能的情况

1. **完全一致**：如果两个版本的序列化格式完全兼容，所有测试的大小应该相同
2. **部分差异**：某些容器类型的实现可能有优化，导致特定模式下大小不同
3. **系统性差异**：如果序列化格式有重大变更，可能所有测试都有差异

### 输出示例

```
======================================================================
Roaring Bitmap v1 vs v2 - getSizeInBytes Comparison
======================================================================

[Test 1: Empty Bitmap]
  Elements: 0
  Portable: Yes
  V1 getSizeInBytes: 8 bytes
  V2 getSizeInBytes: 8 bytes
  Difference: 0 bytes (0.00%)
  Match: ✓ YES

[Test 4: Large Dense Range (0-9999)]
  Elements: 10000
  Portable: Yes
  V1 getSizeInBytes: 1234 bytes
  V2 getSizeInBytes: 1234 bytes
  Difference: 0 bytes (0.00%)
  Match: ✓ YES
```

## 故障排除

### 问题 1: CMake 配置失败

**症状**：`CMake Error: ...`

**解决方案**：
- 确保 CMake 版本 >= 3.15
- 检查 v1 和 v2 目录是否存在且包含 CMakeLists.txt

### 问题 2: 编译错误

**症状**：找不到头文件或链接错误

**解决方案**：
- 清理构建目录：`rm -rf build && mkdir build`
- 重新配置：`cd build && cmake ..`

### 问题 3: 运行时错误

**症状**：程序崩溃或输出异常

**解决方案**：
- 使用 Debug 模式重新编译：`cmake -DCMAKE_BUILD_TYPE=Debug ..`
- 使用 gdb 调试：`gdb ./main`

## 扩展测试

如果需要添加自定义测试场景，可以在 main.cpp 中添加：

```cpp
// 自定义测试
{
    std::vector<uint64_t> values;
    // 添加你的测试数据
    for (uint64_t i = 0; i < N; ++i) {
        values.push_back(your_pattern(i));
    }
    compare_size(values, "Your Custom Test");
}
```

## 性能考虑

- 本项目主要关注序列化大小对比，不涉及性能测试
- 如需性能对比，可以添加时间测量代码
- 建议使用 Release 模式进行性能测试

## 参考资料

- Roaring Bitmap 官方文档：https://github.com/RoaringBitmap/CRoaring
- v1 版本信息：0.3.1
- v2 版本信息：4.4.2

## 许可证

本对比项目遵循 Roaring Bitmap 的原始许可证。
