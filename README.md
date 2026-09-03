# 本地文件搜索工具

## 项目简介

这是一个使用 C++ 编写的命令行文件搜索工具。

v0.1 的目标是递归扫描用户指定的目录，保存文件名、完整路径和文件大小，并支持按文件名关键字搜索。

## 当前进度

目前已完成：

- 建立可编译的 C++ 项目
- 定义 `FileInfo` 数据结构
- 使用 `std::vector<FileInfo>` 保存文件信息
- 完成文件信息容器的基本输出测试

尚未完成：

- 递归扫描目录
- 文件名关键字搜索
- 命令行交互循环
- 文件系统错误处理

## 开发环境

- 操作系统：Windows 11
- IDE：Visual Studio 2026
- 编译器：MSVC 19.51.36252
- C++ 标准：C++17

## 构建方法

### 方法一：使用 Visual Studio

1. 使用 Visual Studio 打开 `文件搜索项目练习.slnx`。
2. 选择 `Debug` 和 `x86` 配置。
3. 执行“生成解决方案”。
4. 运行生成的程序。

### 方法二：使用命令行

在 Visual Studio Developer PowerShell 或 Developer Command Prompt 中进入项目目录，然后执行：

```powershell
cl /std:c++17 /EHsc /utf-8 main.cpp src\fileinfo.cpp /Fe:file-search.exe
```

运行程序：

```powershell
.\file-search.exe
```

## 当前测试

程序会创建三条模拟文件数据，将其加入 `std::vector<FileInfo>`，然后依次输出。

预期输出：

```text
text1 "/text1" 123
text2 "/text2" 1234
text3 "/text3" 1235
```

`std::filesystem::path` 输出时可能自动在路径两侧添加引号，这是正常行为。
```text
file-searcsh-tool/
├── main.cpp
├── src/
│   ├── fileinfo.h
│   └── fileinfo.cpp
├── README.md
└── 文件搜索项目练习.slnx
```

各文件用途：

- `main.cpp`：程序入口和当前阶段的测试代码
- `src/fileinfo.h`：定义文件信息结构
- `src/fileinfo.cpp`：后续存放文件信息相关实现
- `README.md`：项目构建和使用说明

## 已知限制

当前版本只是 T1/T2 阶段的基础结构，还不能扫描真实目录或执行文件搜索。

完整的 v0.1 功能将在后续任务中逐步实现。