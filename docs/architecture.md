# 架构概览

PL/0 JIT 编译器采用经典的多阶段编译器架构，从源代码到机器码执行共经历 5 个主要阶段。

## 系统架构图

```
┌─────────────┐
│  源代码      │  samples/fib.pas
│  (.pas)     │
└──────┬──────┘
       │
       │ 1. 词法分析 & 语法分析
       ↓
┌─────────────┐
│  AST        │  cpp-peglib 解析器
│  语法树     │  grammar.h
└──────┬──────┘
       │
       │ 2. 语义分析
       ↓
┌─────────────┐
│  符号表      │  SymbolTableBuilder
│  + 作用域    │  symbol_table.cc
└──────┬──────┘
       │
       │ 3. 代码生成
       ↓
┌─────────────┐
│  LLVM IR    │  JITCompiler
│  中间表示    │  jit_compiler.cc
└──────┬──────┘
       │
       │ 4. JIT 编译
       ↓
┌─────────────┐
│  机器码      │  LLVM MCJIT
│  Native     │  ExecutionEngine
└──────┬──────┘
       │
       │ 5. 执行
       ↓
┌─────────────┐
│  输出结果    │  stdout
└─────────────┘
```

## 核心模块

### 1. Grammar Module (语法定义)
- **文件**: `include/grammar.h`
- **职责**: 定义 PL/0 的 PEG 语法规则
- **技术**: cpp-peglib 解析器生成器
- **输入**: 无
- **输出**: 语法规则供解析器使用

### 2. Parser (解析器)
- **文件**: `src/main.cc` (使用 cpp-peglib)
- **职责**: 词法分析 + 语法分析
- **输入**: PL/0 源代码文本
- **输出**: 抽象语法树 (AST)
- **错误**: 语法错误检测和报告

### 3. AST Module (抽象语法树)
- **文件**: `include/ast.h`, `src/ast.cc`
- **职责**: 
  - 定义 AST 节点结构
  - 管理符号作用域
  - 提供 AST 遍历辅助函数
- **数据结构**:
  - `AstPL0`: AST 节点类型
  - `SymbolScope`: 作用域管理
  - `Annotation`: 节点注解

### 4. Symbol Table Module (符号表)
- **文件**: `include/symbol_table.h`, `src/symbol_table.cc`
- **职责**: 
  - 语义分析
  - 符号表构建
  - 作用域检查
  - 变量/常量/过程声明检查
- **输入**: AST
- **输出**: 带符号信息的 AST
- **错误**: 语义错误（未定义变量、重复定义等）

### 5. JIT Compiler Module (JIT 编译器)
- **文件**: `include/jit_compiler.h`, `src/jit_compiler.cc`
- **职责**: 
  - LLVM IR 代码生成
  - 异常处理代码生成
  - JIT 编译和执行
- **输入**: 带符号表的 AST
- **输出**: 可执行的机器码
- **技术**: LLVM IR Builder API

### 6. Utils Module (工具)
- **文件**: `include/utils.h`, `src/utils.cc`
- **职责**: 通用工具函数
- **功能**: 错误消息格式化

## 编译流程详解

### 阶段 1: 词法和语法分析

```cpp
// main.cc
parser parser(grammar);
parser.enable_ast<AstPL0>();
parser.parse_n(source.data(), source.size(), ast, path);
```

**过程**:
1. 读取源文件到内存
2. PEG 解析器按语法规则匹配
3. 构建抽象语法树
4. 每个语法规则对应一个 AST 节点

**示例** - 源码到 AST:
```pascal
VAR x;
BEGIN
  x := 10
END.
```

→ AST 结构:
```
program
└── block
    ├── const (empty)
    ├── var
    │   └── ident "x"
    ├── procedure (empty)
    └── statement
        └── assignment
            ├── ident "x"
            └── expression
                └── number "10"
```

### 阶段 2: 语义分析

```cpp
SymbolTableBuilder::build_on_ast(ast);
```

**过程**:
1. 遍历 AST
2. 为每个 block 创建 SymbolScope
3. 记录常量、变量、过程
4. 检查名称冲突
5. 检查未定义引用
6. 分析闭包和自由变量

**符号表结构**:
```cpp
SymbolScope {
  constants: {"pi" -> 3, "max" -> 100}
  variables: {"x", "y", "result"}
  procedures: {"factorial" -> block_node}
  free_variables: {"x"}  // 被内层过程引用的外层变量
}
```

### 阶段 3: LLVM IR 生成

```cpp
JITCompiler::compile(ast);
```

**过程**:
1. 创建 LLVM Module
2. 生成运行时库函数 (`out`, `printf`)
3. 生成程序入口函数
4. 遍历 AST 生成 IR 代码
5. 添加异常处理代码
6. 验证生成的 IR

**IR 示例** - `x := 10`:
```llvm
%x = alloca i32              ; 分配变量
store i32 10, i32* %x        ; 存储值
%0 = load i32, i32* %x       ; 读取值
call void @out(i32 %0)       ; 输出
```

### 阶段 4: JIT 编译

```cpp
ExecutionEngine* ee = EngineBuilder(module).create();
```

**过程**:
1. LLVM 优化 IR
2. 目标机器代码生成
3. 加载到内存
4. 符号解析和重定位

### 阶段 5: 执行

```cpp
ee->runFunction(main_function, {});
```

**过程**:
1. 调用生成的 main 函数
2. 执行机器码
3. 异常处理（如果需要）
4. 输出结果

## 数据流图

```
源文件 (hello.pas)
    ↓
[PEG Parser]
    ↓
AST (内存树结构)
    ↓
[Symbol Builder] ← grammar.h
    ↓
AST + SymbolScope
    ↓
[JIT Compiler] ← LLVM API
    ↓
LLVM Module (IR)
    ↓
[LLVM Optimizer]
    ↓
优化的 IR
    ↓
[LLVM CodeGen]
    ↓
机器码 (内存中)
    ↓
[Execute]
    ↓
stdout (输出)
```

## 关键设计决策

### 1. 使用 PEG 而非传统 BNF
**原因**: 
- PEG 无歧义
- 不需要独立的词法分析器
- cpp-peglib 直接生成 AST

### 2. 分离符号表构建和代码生成
**原因**:
- 清晰的职责分离
- 易于错误检测
- 支持多遍编译

### 3. 使用 LLVM JIT 而非解释执行
**原因**:
- 性能优异（比解释器快 10 倍+）
- 自动优化
- 跨平台支持
- 成熟的工具链

### 4. 闭包通过参数传递实现
**原因**:
- 避免复杂的栈帧管理
- LLVM 函数参数机制天然支持
- 简化实现

### 5. C++ 异常处理机制
**原因**:
- 利用 LLVM 的异常处理支持
- 与 C++ 运行时兼容
- 支持带消息的异常

## 性能特性

### 编译时优化
- **常量折叠**: 编译时计算常量表达式
- **死代码消除**: 移除永不执行的代码
- **内联**: 小函数自动内联

### 运行时特性
- **JIT 编译**: 一次编译，重复执行（如果需要）
- **寄存器分配**: LLVM 优化寄存器使用
- **指令选择**: 针对目标平台的最优指令

### 内存模型
- **栈分配**: 所有局部变量在栈上
- **无垃圾回收**: 简单的生命周期管理
- **零堆分配**: 运行时无动态内存

## 扩展点

### 易于扩展的部分
1. **新语法特性**: 修改 `grammar.h`
2. **新语句类型**: 在 `JITCompiler` 添加 `compile_xxx` 方法
3. **新运行时函数**: 在 `compile_libs()` 中添加
4. **新优化**: 利用 LLVM Pass Manager

### 难以扩展的部分
1. **类型系统**: 当前只支持整数
2. **数组/结构体**: 需要重新设计符号表
3. **垃圾回收**: 需要添加运行时支持
4. **模块系统**: 需要设计链接机制

## 错误处理层次

### 编译时错误
1. **语法错误** (Parser)
   - 位置: 解析阶段
   - 示例: 缺少分号、括号不匹配

2. **语义错误** (SymbolTableBuilder)
   - 位置: 符号表构建
   - 示例: 未定义变量、重复声明

3. **代码生成错误** (JITCompiler)
   - 位置: IR 生成
   - 示例: 符号解析失败

### 运行时错误
1. **除零异常**
   - 捕获并显示错误消息
   - 正常退出程序

2. **栈溢出**
   - 系统级错误
   - 深度递归导致

## 依赖关系图

```
main.cc
  ├─→ grammar.h (无依赖)
  ├─→ utils.h
  │     └─→ utils.cc
  ├─→ ast.h
  │     ├─→ peglib.h (第三方)
  │     ├─→ utils.h
  │     └─→ ast.cc
  ├─→ symbol_table.h
  │     ├─→ ast.h
  │     └─→ symbol_table.cc
  └─→ jit_compiler.h
        ├─→ ast.h
        ├─→ LLVM (第三方)
        └─→ jit_compiler.cc
```

## 下一步

- 详细了解 [编译流程](compilation-pipeline.md)
- 查看 [模块设计](module-design.md)
- 学习 [LLVM IR 详解](llvm-ir-guide.md)
- 阅读 [API 文档](api/ast.md)
