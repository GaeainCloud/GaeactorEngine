
# Gaeactor Engine

多模块的 C++ 多智能体/场景仿真与通信引擎框架（Gaeactor）。此仓库以 CMake 为构建入口，包含若干互相依赖的子项目（例如 `gaeactor`、`gaeactor-engine`、`gaeactor-comm`、`gaeactor-transmit`、`gaeactor-viewer` 等），并通过 `cmake/` 目录下的模块管理第三方库查找与配置。

本 README 提供项目概览、依赖与快速构建说明、代码组织与贡献指南，方便开发者在 Windows 与 Linux 环境下开始开发或二次构建。

---

## 主要功能概览

- 模块化架构：多个子项目按功能划分（通信、传输、环境、智能体、可视化、测试等）。
- 跨平台构建：基于 CMake，支持 MSVC、GCC、Clang。
- 集成第三方库：`cmake/` 下包含大量第三方库的查找/配置脚本（图形、网络、数学、资源读取等）。

---

## 先决条件（建议）

- `git`
- `cmake`（建议 3.15 及以上）
- Windows: Visual Studio 2019/2022（含 MSVC 工具集）或等效编译器
- Linux: `gcc` / `clang`、`make` 或 `ninja`
- Bash（仓库提供的构建脚本为 shell 脚本，在 Windows 上可使用 Git Bash / MSYS2 / WSL）

注意：仓库中的 `cmake/` 模块会查找或帮助构建第三方依赖。视具体模块，有时需要预先在系统上安装开发库或运行仓库内的准备脚本（见下文）。

---

## 快速构建（示例）

仓库根目录提供了两个脚本：`build-sdk-src-lnx.sh`（Linux）和 `build-sdk-src-win.sh`（Windows）。这些脚本通常用于准备或构建第三方 SDK/依赖（具体行为以脚本实现为准）。下面给出典型的构建流程示例：

- 在 Linux / WSL：

```bash
# 可选：准备依赖（根据仓库脚本）
bash ./build-sdk-src-lnx.sh

# 创建构建目录并构建
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

- 在 Windows（使用 Git Bash / MSYS / WSL，或在 PowerShell/Developer Command Prompt 中按需调整命令）：

```bash
# 可选：准备依赖（Git Bash 下运行）
bash ./build-sdk-src-win.sh

# 使用适当的 CMake 生成器，例如 Visual Studio 2019 x64
mkdir -p build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -- /m
```

提示：若只需要编译某一子模块（例如 `gaeactor-engine`），可进入该子目录并按相同方式创建 `build`、运行 `cmake ..` 与 `cmake --build`。

若遇到依赖项缺失，先检查 `cmake/` 下对应的模块文件（例如 `lib*.cmake`），或在系统上安装所需的开发包。

---

## 顶层 CMake（依赖说明）

顶层 `CMakeLists.txt` 会对多个子目录调用 `add_subdirectory(...)`，并通过 `add_dependencies(...)` 声明模块间的构建依赖，以确保按正确顺序构建。

示例（摘录）：

```cmake
add_subdirectory(gaeactor-comm)
add_subdirectory(gaeactor-transmit)
add_subdirectory(gaeactor-environment)
add_subdirectory(gaeactor-engine)
add_dependencies(gaeactor-transmit gaeactor-comm)
add_dependencies(gaeactor-engine gaeactor-transmit gaeactor)
```

因此推荐使用顶层构建方式（从仓库根目录运行 `cmake`），以保证子模块之间的依赖关系正确处理。

---

## 常见目录（概要）

- `CMakeLists.txt`          : 顶层 CMake 文件
- `cmake/`                  : 自定义 CMake 模块与第三方库配置
- `gaeactor/`               : 框架核心模块
- `gaeactor-engine/`        : 引擎实现
- `gaeactor-comm/`          : 通信模块
- `gaeactor-transmit/`      : 传输/适配层
- `gaeactor-viewer/`        : 可视化/查看器模块
- `gaeactor-test/`          : 测试、示例或演示程序
- `build-sdk-src-*.sh`      : 依赖准备 / SDK 构建脚本（Windows / Linux）

仓库还有其它子模块与演示目录（例如 `TierAirlineAirPortSimulation/` 等），请根据需要进入相应目录查看子模块文档与 `CMakeLists.txt`。

---

## 调试与开发建议

- 初次构建：先运行 `build-sdk-src-*.sh`（如果项目依赖脚本的自动构建行为）。
- 使用 `-DCMAKE_BUILD_TYPE=Debug` 开启调试符号。
- 在 Windows 上可直接使用 Visual Studio 打开 CMake 生成的解决方案进行断点调试。
- 如果需要减少构建时间，可在 `cmake` 中只启用需要的子模块或目标。

---

## 测试

仓库包含测试与演示目标（例如 `gaeactor-test`）。构建完成后，你可以在 `build` 目录中直接运行相应可执行文件，或使用 `cmake --build . --target <target>` 指定目标来单独构建测试程序。

---

## 贡献与 PR 指引

- Fork 仓库并创建 feature 分支。
- 完成后发起 Pull Request，并在 PR 描述中写明变更内容与兼容性影响（若有）。
- 提交信息应清晰简洁；涉及接口或行为变更请提供对应的说明或升级引导。

如需，我可以帮助添加：CI 配置（GitHub Actions / GitLab CI）、贡献模板、或为某个子模块写更详细的快速开始指南。

---

## 许可与作者

- 作者 / 维护者：GaeainCloud 组织
- 许可：当前仓库根目录未包含明确的 `LICENSE` 文件。请补充项目的许可文件（如 MIT / Apache-2.0 / BSD 等），我可以帮你添加合适的许可模板。

---

## 联系与支持

如需帮助，请通过仓库 Issue 提问或联系仓库维护团队（组织内部渠道）。

---

如果你希望我把 README 翻译为英文、添加 CI 配置、或为某一子项目（例如 `gaeactor-engine`）写更详细的“快速运行示例”，告诉我你想要的目标和平台，我会继续完善。
