# 快速开始指南

## 一键构建和运行

```bash
cd /root/test
./build.sh
./build/main
```

## 手动构建

```bash
cd /root/test
mkdir build && cd build
cmake ..
make -j$(nproc)
./main
```

## 项目文件说明

### 核心文件

1. **CMakeLists.txt** - 构建配置文件
   - 分别构建 v1 和 v2 的 Roaring Bitmap 库
   - 创建两个测试库（test_v1_lib 和 test_v2_lib）
   - 链接到 main 可执行文件

2. **main.cpp** - 主程序
   - 通过 C 接口调用 v1 和 v2 的测试函数
   - 对比 getSizeInBytes() 的返回值
   - 包含 12 个不同的测试场景

3. **build.sh** - 自动构建脚本
   - 清理旧的构建目录
   - 配置和编译项目
   - 显示构建结果

### 自动生成的文件

构建过程中会自动生成两个测试源文件：

- **build/test_v1.cpp** - v1 测试函数实现
- **build/test_v2.cpp** - v2 测试函数实现

这两个文件分别包含 v1 和 v2 的头文件，避免了头文件冲突。

## 关键技术点

### 1. 头文件隔离

由于 v1 和 v2 使用相同的头文件名，我们采用以下策略：

```cpp
// test_v1.cpp - 只包含 v1 的头文件
#include "roaring/roaring64map.hh"
extern "C" size_t test_roaring_v1(...) { ... }

// test_v2.cpp - 只包含 v2 的头文件
#include "roaring/roaring64map.hh"
extern "C" size_t test_roaring_v2(...) { ... }

// main.cpp - 不包含任何 roaring 头文件，只声明 C 接口
extern "C" {
    size_t test_roaring_v1(...);
    size_t test_roaring_v2(...);
}
```

### 2. 独立编译

```cmake
# v1 测试库使用 v1 的头文件路径
target_include_directories(test_v1_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/v1/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v1/cpp
)

# v2 测试库使用 v2 的头文件路径
target_include_directories(test_v2_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/v2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v2/cpp
)
```

### 3. C 接口隔离

使用 `extern "C"` 避免 C++ 名称修饰（name mangling）导致的符号冲突。

## 测试场景详解

| 测试编号 | 场景描述 | 目的 |
|---------|---------|------|
| Test 1  | 空位图 | 测试最小序列化大小 |
| Test 2  | 单个元素 | 测试最简单情况 |
| Test 3  | 小密集范围 (0-99) | 测试小规模密集数据 |
| Test 4  | 大密集范围 (0-9999) | 测试大规模密集数据压缩 |
| Test 5  | 稀疏数据 (间隔1000) | 测试稀疏数据处理 |
| Test 6  | 极稀疏数据 (间隔1M) | 测试极端稀疏情况 |
| Test 7  | 混合模式 | 测试密集+稀疏组合 |
| Test 8  | 大数值 | 测试高32位数据 |
| Test 9  | 多桶 | 测试跨多个32位桶 |
| Test 10 | 非便携格式 | 测试非便携序列化 |
| Test 11 | 伪随机模式 | 测试随机分布数据 |
| Test 12 | 2的幂次 | 测试特殊稀疏模式 |

## 输出解读

```
[Test 4: Large Dense Range (0-9999)]
  Elements: 10000                    # 元素数量
  Portable: Yes                      # 是否使用便携格式
  V1 getSizeInBytes: 1234 bytes     # v1 序列化大小
  V2 getSizeInBytes: 1234 bytes     # v2 序列化大小
  Difference: 0 bytes (0.00%)       # 差异（绝对值和百分比）
  Match: ✓ YES                       # 是否匹配
```

### 差异分析

- **Match: ✓ YES** - 两个版本序列化大小完全一致
- **Match: ✗ NO** - 存在差异，可能原因：
  - 容器实现优化
  - 压缩算法改进
  - 序列化格式变更
  - Run-length encoding 策略不同

## 常见问题

### Q1: 为什么不能直接在 main.cpp 中包含两个版本的头文件？

A: 因为两个版本的头文件名称相同（`roaring/roaring64map.hh`），且都定义了相同的类名（`roaring::Roaring64Map`），直接包含会导致符号冲突。

### Q1.5: CMake 如何解决目标名称冲突？

A: v1 和 v2 都会创建名为 "roaring" 的库目标，导致冲突。我们的解决方案：
- v1：通过设置 `ROARING_LIB_NAME=roaring_v1` 来命名
- v2：在配置时动态修补 v2/src/CMakeLists.txt，将目标名从 "roaring" 改为 "roaring_v2"
- 原始文件会被备份为 `v2/src/CMakeLists.txt.original`
- 使用 `./clean.sh` 可以恢复原始文件

### Q2: 如何添加自定义测试？

A: 在 main.cpp 的 main() 函数中添加：

```cpp
{
    std::vector<uint64_t> values;
    // 添加你的测试数据
    compare_size(values, "My Custom Test");
}
```

### Q3: 如何只测试特定场景？

A: 注释掉 main.cpp 中不需要的测试代码块。

### Q4: 构建失败怎么办？

A:
1. 清理构建目录：`rm -rf build`
2. 检查 v1 和 v2 目录是否完整
3. 确保 CMake 版本 >= 3.15
4. 查看详细错误信息

## 性能提示

- 使用 Release 模式获得最佳性能：`cmake -DCMAKE_BUILD_TYPE=Release ..`
- 使用 Debug 模式便于调试：`cmake -DCMAKE_BUILD_TYPE=Debug ..`
- 并行编译加速：`make -j$(nproc)`

## 下一步

- 查看 [README.md](README.md) 了解详细技术文档
- 查看 [README_BUILD.md](README_BUILD.md) 了解更多构建选项
- 修改 main.cpp 添加自定义测试场景
