# CMake 目标名称冲突解决方案

## 问题描述

当同时编译 v1 和 v2 的 Roaring Bitmap 库时，会遇到以下 CMake 错误：

```
CMake Error: add_library cannot create target "roaring" because another
target with the same name already exists.
```

这是因为：
1. v1 和 v2 都创建了名为 `roaring` 的库目标
2. v1 和 v2 都创建了 `roaring-headers` 和 `roaring-headers-cpp` 接口库

## 解决方案

我们的 CMakeLists.txt 采用**动态修补**策略，在配置阶段自动修改 v2 的 CMakeLists.txt 文件：

### 1. v1 库命名

v1 使用 `ROARING_LIB_NAME` 变量，我们设置为 `roaring_v1`：

```cmake
set(ROARING_LIB_NAME "roaring_v1" CACHE STRING "v1 library name" FORCE)
add_subdirectory(v1 ${CMAKE_BINARY_DIR}/v1_build)
```

### 2. v2 库修补

对 v2 的两个 CMakeLists.txt 文件进行修补：

#### v2/CMakeLists.txt（主文件）
- `roaring-headers` → `roaring-headers-v2`
- `roaring-headers-cpp` → `roaring-headers-cpp-v2`

#### v2/src/CMakeLists.txt（源文件）
- `roaring` → `roaring_v2`
- `roaring-headers` → `roaring-headers-v2`
- `roaring-headers-cpp` → `roaring-headers-cpp-v2`
- `roaring-config` → `roaring_v2-config`
- 等等...

### 3. 自动备份

原始文件会被自动备份：
- `v2/CMakeLists.txt.original`
- `v2/src/CMakeLists.txt.original`

### 4. 恢复原始文件

运行清理脚本即可恢复：

```bash
./clean.sh
```

## 最终目标结构

```
v1 库:
  - roaring_v1 (静态库)
  - roaring-headers (接口库)
  - roaring-headers-cpp (接口库)

v2 库:
  - roaring_v2 (静态库)
  - roaring-headers-v2 (接口库)
  - roaring-headers-cpp-v2 (接口库)

测试库:
  - test_v1_lib → 链接 roaring_v1
  - test_v2_lib → 链接 roaring_v2

主程序:
  - main → 链接 test_v1_lib 和 test_v2_lib
```

## 使用流程

### 首次构建

```bash
./build.sh
```

或手动：

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./main
```

### 清理和重新构建

```bash
./clean.sh
./build.sh
```

## 技术细节

### 为什么不使用命名空间？

CMake 的目标名称是全局的，即使在不同的子目录中定义，也会产生冲突。命名空间只能解决 C++ 符号冲突，无法解决 CMake 目标冲突。

### 为什么要修补文件？

v2 的 CMakeLists.txt 硬编码了目标名称，没有提供变量来自定义。动态修补是最简洁的解决方案，不需要手动维护修改后的文件。

### 修补是否安全？

是的，因为：
1. 原始文件会被备份
2. 修补只在配置阶段进行，不影响源代码
3. 可以随时通过 `./clean.sh` 恢复
4. 修补后的文件不会被提交到版本控制

### 如果 v2 更新了怎么办？

如果 v2 的 CMakeLists.txt 结构发生重大变化，可能需要更新主 CMakeLists.txt 中的修补逻辑。但由于我们使用的是简单的字符串替换，大多数情况下都能正常工作。

## 故障排除

### 问题：CMake 仍然报告目标冲突

**解决方案**：
```bash
./clean.sh
rm -rf build
./build.sh
```

### 问题：找不到 roaring_v2 目标

**原因**：修补可能失败

**解决方案**：
1. 检查 v2/src/CMakeLists.txt 是否存在
2. 查看 CMake 输出中的错误信息
3. 手动检查修补后的文件

### 问题：链接错误 - multiple definition

**原因**：v1 和 v2 的 C 函数有相同的符号名称（如 `roaring_bitmap_create_with_capacity`）

**解决方案**：
在 main 的链接选项中添加 `-Wl,--allow-multiple-definition`：
```cmake
target_link_options(main PRIVATE "-Wl,--allow-multiple-definition")
```

这是安全的，因为：
- test_v1_lib 和 test_v2_lib 完全封装了各自的 roaring 库
- main.cpp 只调用 extern "C" 的测试函数，不直接使用 roaring 符号
- 两个版本的符号虽然名称相同，但被隔离在不同的测试库中

### 问题：v1 库名称不正确

**原因**：v1 的 CMakeLists.txt 硬编码了 `set(ROARING_LIB_NAME roaring)`

**解决方案**：
修补 v1/CMakeLists.txt，将 `roaring` 替换为 `roaring_v1`

## 总结

这个解决方案通过动态修补 v2 的 CMakeLists.txt 文件，成功解决了目标名称冲突问题，同时保持了：
- ✅ 源代码不变
- ✅ 可以随时恢复
- ✅ 自动化构建
- ✅ 清晰的目标结构
- ✅ 易于维护
