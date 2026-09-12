# 本地文件搜索工具

## 项目简介

这是一个使用 C++ 编写的命令行文件搜索工具。

v0.2 的目标是支持关键字、拓展名、大小的组合搜索。

## 当前进度

目前已完成：

- 建立可编译的 C++ 项目
- 定义 `FileInfo` 数据结构
- 使用 `std::vector<FileInfo>` 保存文件信息
- 完成文件信息容器的基本输出测试
- 完成递归扫描真实目录
- 完成单次的简单文件搜索
- 命令行交互循环
- 组合搜索信息的保存
- 组合搜索信息的输入
- 组合搜索信息的解析，包括识别引号、判断数字是否溢出等
- 组合搜索
- 组合搜索功能添加至主循环

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
cl /std:c++17 /EHsc /utf-8 main.cpp src\fileinfo.cpp src\search.cpp /Fe:file-search.exe
```

运行程序：

```powershell
.\file-search.exe
```

## 当前测试
### 搜索信息解析测试
#### 测试程序构建方法
在 Visual Studio Developer PowerShell 或 Developer Command Prompt 中进入项目目录，然后执行：

```powershell
cl /std:c++17 /EHsc /utf-8 text\v0.2_text.cpp src\fileinfo.cpp src\search.cpp /Fe:text\v0.2_text.exe
```
运行程序：
```powershell
.\text\v0.2_text.exe
```
预期输出(省略PASS输出)：
```
127/127 tests passed.
```
## 交互样例
```text
Scan directory:E:/光标
Found 21 files.

Enter any to leave a condition unrestricted.
Sizes are in bytes.
Enter exit at the keyword prompt to quit.
Use double quotes for literal text, e.g. "any" or "exit".

Search:
keyword: 必看

search_extname: .txt

min_size: 0

max_size: any

Find 1 files.
"E:/光标\\必看.txt"
Search:
keyword: exit
```
输入四个字段后会进行检查，非法输入会要求重新输入，如：
```text
Scan directory:E:/光标
Found 21 files.

Enter any to leave a condition unrestricted.
Sizes are in bytes.
Enter exit at the keyword prompt to quit.
Use double quotes for literal text, e.g. "any" or "exit".

Search:
keyword:

search_extname: .txt

min_size: any

max_size: any

Error:keyword is empty
input keyword again:
```
## 项目结构
```
file-searcsh-tool/
├── main.cpp
├── src/
│   ├── search.h
│   ├── search.cpp
│   ├── fileinfo.h
│   └── fileinfo.cpp
├── README.md
├── text/
│   └── v0.2_text.cpp
└── 文件搜索项目练习.slnx
```

各文件用途：

- `main.cpp`：程序入口和当前阶段的测试代码
- `src/fileinfo.h`：定义文件信息结构
- `src/fileinfo.cpp`：存放文件信息相关实现
- `src/search.h`：定义搜索信息结构
- `src/search.cpp`：存放搜索信息相关实现
- `README.md`：项目构建和使用说明
- `text`:测试文件

## 项目状态

v0.2 核心功能已完成。

## 已知限制

- 搜索区分大小写
- 不搜索文件内容
- 不跟随符号链接
