# 快速开始

本指南将帮助你快速上手 PL/0 JIT 编译器。

## 系统要求

### 必需依赖
- **操作系统**: Linux, macOS
- **编译器**: Clang++ (支持 C++17)
- **LLVM**: 13.0 或更高版本
- **构建工具**: Make

### Debian/Ubuntu 安装依赖

```bash
sudo apt-get update
sudo apt-get install -y clang llvm llvm-dev make
```

### macOS 安装依赖

```bash
brew install llvm
```

## 编译项目

### 1. 克隆/获取代码

```bash
cd /path/to/pl0-jit-compiler
```

### 2. 编译

```bash
make
```

编译成功后，会在当前目录生成 `pl0` 可执行文件。

### 3. 验证安装

```bash
./pl0 samples/square.pas
```

预期输出：
```
1
4
9
16
25
36
49
64
81
100
```

## 第一个程序

### 创建源文件

创建文件 `hello.pas`：

```pascal
VAR x;

BEGIN
   x := 42;
   ! x
END.
```

### 运行程序

```bash
./pl0 hello.pas
```

输出：
```
42
```

## 示例程序

项目包含多个示例程序在 `samples/` 目录：

### 1. 平方数 (square.pas)
```bash
./pl0 samples/square.pas
```
计算并输出 1-10 的平方。

### 2. 斐波那契数列 (fib.pas)
```bash
./pl0 samples/fib.pas
```
计算并输出斐波那契数列。

### 3. 最大公约数 (gcd.pas)
```bash
./pl0 samples/gcd.pas
```
计算两个数的最大公约数。

### 4. 嵌套函数 (nested-functions.pas)
```bash
./pl0 samples/nested-functions.pas
```
演示嵌套过程调用。

### 5. 除零异常 (divide-by-zero.pas)
```bash
./pl0 samples/divide-by-zero.pas
```
演示异常处理机制。

## 基本命令

### 编译项目
```bash
make
```

### 清理构建文件
```bash
make clean
```

### 运行基准测试
```bash
make bench
```

### 查看帮助
```bash
make help
```

## PL/0 语言基础

### 程序结构

```pascal
[CONST 常量定义;]
[VAR 变量定义;]
[PROCEDURE 过程定义;]
BEGIN
   语句序列
END.
```

### 基本语法

#### 常量定义
```pascal
CONST pi = 3, max = 100;
```

#### 变量定义
```pascal
VAR x, y, result;
```

#### 赋值语句
```pascal
x := 10;
result := x * 2 + 5;
```

#### 输出语句
```pascal
! x;           // 方式 1
out x;         // 方式 2
write x;       // 方式 3
```

#### 条件语句
```pascal
IF x > 10 THEN
   ! x;
```

#### 循环语句
```pascal
WHILE x < 100 DO
BEGIN
   ! x;
   x := x + 1
END;
```

#### 过程定义和调用
```pascal
PROCEDURE printNum;
BEGIN
   ! x
END;

CALL printNum;
```

## 性能对比

运行基准测试：

```bash
make bench
```

典型结果（斐波那契数列计算）：
- **Python**: ~1.15 秒
- **Ruby**: ~1.38 秒
- **PL/0 JIT**: ~0.10 秒 ⚡

PL/0 JIT 编译器比解释型语言快 **10-13 倍**！

## 常见问题

### Q: 编译失败，提示找不到 llvm-config
**A**: 确保 LLVM 已正确安装并在 PATH 中。尝试 `which llvm-config`。

### Q: 运行时出现 "can't open the source file"
**A**: 检查文件路径是否正确，确保文件存在。

### Q: 程序没有输出
**A**: 确保你的 PL/0 程序中有输出语句 (`!`, `out`, 或 `write`)。

### Q: 如何查看生成的 LLVM IR？
**A**: 修改 `src/jit_compiler.cc` 中的 `run()` 函数，取消注释 `jit.dump()` 行。

## 下一步

- 学习完整的 [PL/0 语言教程](pl0-language.md)
- 了解 [编译器架构](architecture.md)
- 查看 [API 文档](api/ast.md)
- 阅读 [LLVM IR 详解](llvm-ir-guide.md)

## 获取帮助

遇到问题？
1. 查看 [FAQ](faq.md)
2. 阅读 [调试技巧](debugging.md)
3. 在 GitHub 上提 Issue
