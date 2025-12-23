PL/0 JIT compiler
=================

A tiny [PL/0](https://en.wikipedia.org/wiki/PL/0) JIT compiler in less than 900 LOC with LLVM and PEG parser which handles 'Divide by Zero'.

## 📚 Documentation

**完整文档请查看 [docs/](docs/) 目录**

- **[快速开始](docs/getting-started.md)** - 安装和第一个程序
- **[PL/0 语言教程](docs/pl0-language.md)** - 完整的语言语法和示例
- **[架构概览](docs/architecture.md)** - 系统设计和模块说明
- **[LLVM IR 详解](docs/llvm-ir-guide.md)** - 中间表示深入解析
- **[异常处理机制](docs/exception-handling.md)** - 除零检查实现
- **[API 参考](docs/api/)** - 完整的 API 文档

## Features

- ✨ **完整的 PL/0 实现** - 支持常量、变量、过程、控制流
- ⚡ **JIT 编译** - 使用 LLVM，性能比 Python/Ruby 快 10+ 倍
- 🛡️ **异常处理** - 自动除零检查，使用 C++ 异常机制
- 🏗️ **模块化设计** - 清晰的代码结构，易于维护和扩展
- 📖 **完善文档** - 详细的文档和代码注释

## Project Structure

```
pl0-jit-compiler/
├── include/              # 头文件
│   ├── ast.h            # AST 定义和符号作用域
│   ├── grammar.h        # PL/0 语法
│   ├── jit_compiler.h   # JIT 编译器
│   ├── symbol_table.h   # 符号表构建
│   └── utils.h          # 工具函数
├── src/                 # 源文件
│   ├── ast.cc
│   ├── jit_compiler.cc
│   ├── main.cc
│   ├── symbol_table.cc
│   └── utils.cc
├── docs/                # 文档
├── samples/             # 示例程序
├── vendor/              # 第三方库
└── Makefile            # 构建系统
```

Library dependencies:

  * Parse and AST build: [cpp-peglib](https://github.com/yhirose/cpp-peglib)
  * Code Generation: [LLVM 19.1.1](https://discourse.llvm.org/t/llvm-19-1-7-released/84062)

Build 
---------------
MacOS
```sh
brew install llvm
export PATH="$PATH:/usr/local/opt/llvm/bin"
make
```

Linux
```sh
sudo apt install llvm clang
make
```

Usage
-----

```sh
> cat samples/square.pas
VAR x, squ;

PROCEDURE square;
BEGIN
   squ := x * x
END;

BEGIN
   x := 1;
   WHILE x <= 10 DO
   BEGIN
      CALL square;
      ! squ;
      x := x + 1
   END
END.

> pl0 samples/square.pas
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

Benchmark with Fibonacci number [0, 35)
---------------------------------------

```sh
> make bench
*** Python ***
real	0m8.367s
user	0m8.153s
sys	0m0.129s

*** Ruby ***
real	0m3.064s
user	0m2.916s
sys	0m0.097s

*** PL/0 ***
real	0m0.249s
user	0m0.234s
sys	0m0.008s
```

License
-------

[MIT](https://github.com/yhirose/pl0-jit-compiler/blob/master/LICENSE)
