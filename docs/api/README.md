# API 文档目录

本目录包含所有模块的详细 API 参考文档。

## 模块列表

### [AST 模块](ast.md)
抽象语法树、符号作用域和辅助函数。

**核心类型**:
- `AstPL0` - AST 节点类型
- `SymbolScope` - 符号作用域
- `Annotation` - 节点注解

### [Grammar 模块](grammar.md)
PL/0 语言的 PEG 语法定义。

**内容**:
- 完整的 PEG 语法规则
- 语法规则说明

### [Symbol Table 模块](symbol-table.md)
符号表构建和语义分析。

**核心类**:
- `SymbolTableBuilder` - 符号表构建器

### [JIT Compiler 模块](jit-compiler.md)
LLVM JIT 编译器实现。

**核心类**:
- `JITCompiler` - JIT 编译器

### [Utils 模块](utils.md)
工具函数集合。

**函数**:
- `format_error_message()` - 错误消息格式化

## 使用指南

每个模块文档包含：
1. **概述** - 模块功能简介
2. **类/函数列表** - 完整的 API 列表
3. **使用示例** - 代码示例
4. **注意事项** - 使用建议和限制

## 命名空间

所有模块都在 `pl0` 命名空间下：

```cpp
namespace pl0 {
  // 所有 API
}
```

## 代码风格

### 命名约定
- **类名**: PascalCase (`SymbolScope`)
- **函数名**: snake_case (`get_closest_scope`)
- **变量名**: snake_case (`ast_node`)
- **常量名**: UPPER_CASE 或 snake_case

### 文件组织
- **头文件**: `include/*.h`
- **实现文件**: `src/*.cc`

## 依赖关系

```
utils (无依赖)
  ↑
ast (依赖 utils, peglib)
  ↑
  ├── symbol_table
  └── jit_compiler (额外依赖 LLVM)
```

## 快速查找

### 我想...

**解析源代码**
- 查看 [Grammar 模块](grammar.md)
- 使用 cpp-peglib parser

**构建 AST**
- 查看 [AST 模块](ast.md)
- 使用 `AstPL0` 类型

**进行语义分析**
- 查看 [Symbol Table 模块](symbol-table.md)
- 使用 `SymbolTableBuilder::build_on_ast()`

**生成和执行代码**
- 查看 [JIT Compiler 模块](jit-compiler.md)
- 使用 `JITCompiler::run()`

**格式化错误消息**
- 查看 [Utils 模块](utils.md)
- 使用 `format_error_message()`
