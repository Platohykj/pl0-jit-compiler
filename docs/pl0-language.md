# PL/0 语言完整教程

PL/0 是 Niklaus Wirth 在 1976 年设计的教学用编程语言，用于演示编译器原理。

## 语言概述

PL/0 是 Pascal 的简化子集，具有以下特点：
- **简洁性**: 只有基本的编程结构
- **完整性**: 包含常量、变量、过程、表达式、控制流
- **教学性**: 适合学习编译原理

## 词法元素

### 标识符
- 以小写字母开头
- 可包含小写字母和数字
- 示例: `x`, `sum`, `temp1`, `counter`

### 数字
- 整数常量
- 示例: `0`, `42`, `1234`

### 关键字
```
CONST    VAR       PROCEDURE
BEGIN    END       IF
THEN     WHILE     DO
CALL     ODD
```

### 运算符
```
:=       =         #
<        <=        >
>=       +         -
*        /
```

### 输入输出
```
!        out       write     (输出)
?        in        read      (输入 - 未实现)
```

## 程序结构

### 完整程序框架

```pascal
CONST 常量名 = 值 [, 常量名 = 值]*;

VAR 变量名 [, 变量名]*;

PROCEDURE 过程名;
  [块内容]
BEGIN
  语句
END;

BEGIN
  主程序语句
END.
```

### 结构说明

1. **程序 = 块 + `.`**
2. **块 = [常量声明] + [变量声明] + [过程声明] + 语句**
3. **每个部分都是可选的**

## 声明

### 常量声明

```pascal
CONST 
    pi = 3,
    max = 100,
    min = 0;
```

**规则**:
- 使用 `CONST` 关键字
- 多个常量用逗号分隔
- 分号结尾
- 常量值必须是整数
- 常量不可修改

### 变量声明

```pascal
VAR x, y, z, result;
```

**规则**:
- 使用 `VAR` 关键字
- 多个变量用逗号分隔
- 分号结尾
- 变量初始值未定义

### 过程声明

```pascal
PROCEDURE 过程名;
  [CONST ...]
  [VAR ...]
  [PROCEDURE ...]
BEGIN
  语句
END;
```

**特点**:
- 可以嵌套声明
- 无参数传递
- 通过外部变量通信（闭包）
- 无返回值

## 语句

### 赋值语句

```pascal
x := 10;
y := x * 2 + 5;
result := (x + y) * z;
```

### 过程调用

```pascal
CALL 过程名;
```

示例：
```pascal
PROCEDURE printX;
BEGIN
  ! x
END;

BEGIN
  x := 42;
  CALL printX
END.
```

### 复合语句

```pascal
BEGIN
  语句1;
  语句2;
  语句3
END
```

**注意**: 最后一个语句后面不加分号。

### 条件语句

```pascal
IF 条件 THEN 语句
```

示例：
```pascal
IF x > 10 THEN
  ! x;

IF x = y THEN
BEGIN
  z := 1;
  ! z
END;
```

**限制**: 不支持 `ELSE` 子句。

### 循环语句

```pascal
WHILE 条件 DO 语句
```

示例：
```pascal
VAR i;
BEGIN
  i := 1;
  WHILE i <= 10 DO
  BEGIN
    ! i;
    i := i + 1
  END
END.
```

### 输出语句

三种等价方式：

```pascal
! 表达式;      // 简短形式
out 表达式;    // 明确形式
write 表达式;  // Pascal 风格
```

示例：
```pascal
! 42;
! x + y;
! (a + b) * c;
```

### 空语句

语句可以为空：

```pascal
IF x > 0 THEN
  ;  // 空语句
```

## 表达式

### 算术表达式

```pascal
expression := [+|-] term ([+|-] term)*
term       := factor ([*|/] factor)*
factor     := ident | number | (expression)
```

示例：
```pascal
x + y
x * y + z
(x + y) * (z - w)
-x + y
+100
```

### 比较运算符

```pascal
=    等于
#    不等于
<    小于
<=   小于等于
>    大于
>=   大于等于
```

### 条件表达式

```pascal
ODD 表达式           // 判断奇数
表达式 比较符 表达式  // 比较
```

示例：
```pascal
IF ODD x THEN ! x;
IF x >= y THEN ! x;
IF a + b < c * d THEN ! 0;
```

## 作用域规则

### 词法作用域

```pascal
VAR x;

PROCEDURE outer;
  VAR y;
  
  PROCEDURE inner;
  BEGIN
    ! x;  // 可访问 outer 和全局的变量
    ! y
  END;
BEGIN
  CALL inner
END;
```

### 名称遮蔽

内层声明会遮蔽外层同名声明：

```pascal
VAR x;

PROCEDURE test;
  VAR x;  // 遮蔽外层 x
BEGIN
  x := 10;  // 修改局部 x
  ! x       // 输出: 10
END;

BEGIN
  x := 5;
  CALL test;  // 输出: 10
  ! x         // 输出: 5
END.
```

## 完整示例

### 示例 1: 计算阶乘

```pascal
VAR n, result;

PROCEDURE factorial;
  VAR temp;
BEGIN
  IF n > 1 THEN
  BEGIN
    temp := n;
    n := n - 1;
    CALL factorial;
    result := result * temp
  END
END;

BEGIN
  n := 5;
  result := 1;
  CALL factorial;
  ! result    // 输出: 120
END.
```

### 示例 2: 最大公约数

```pascal
VAR x, y;

PROCEDURE gcd;
  VAR r;
BEGIN
  WHILE y # 0 DO
  BEGIN
    r := x - x / y * y;
    x := y;
    y := r
  END
END;

BEGIN
  x := 48;
  y := 36;
  CALL gcd;
  ! x    // 输出: 12
END.
```

### 示例 3: 嵌套过程

```pascal
VAR x;

PROCEDURE outer;
  VAR y;
  
  PROCEDURE middle;
    VAR z;
    
    PROCEDURE inner;
    BEGIN
      z := x + y;  // 访问所有外层变量
      ! z
    END;
  BEGIN
    z := 0;
    CALL inner
  END;
BEGIN
  y := 20;
  CALL middle
END;

BEGIN
  x := 10;
  CALL outer    // 输出: 30
END.
```

### 示例 4: 除零异常处理

```pascal
VAR x, y;

PROCEDURE divide;
BEGIN
  ! 100 / x
END;

BEGIN
  x := 0;
  CALL divide    // 触发异常: "divide by 0"
END.
```

## 语言限制

### 不支持的特性

1. **ELSE 子句** - 只有 IF-THEN
2. **FOR 循环** - 只有 WHILE
3. **数组** - 只有简单变量
4. **字符串** - 只有整数
5. **函数** - 只有过程（无返回值）
6. **参数传递** - 通过外部变量
7. **浮点数** - 只有整数
8. **输入** - IN/READ 未实现

### 支持的扩展

本实现添加了：
- **除零检查** - 自动检测并抛出异常
- **异常处理** - 使用 C++ 异常机制
- **三种输出方式** - `!`, `out`, `write`

## 语法规则总结

```ebnf
program    ::= block '.'
block      ::= [CONST ident '=' number {',' ident '=' number}* ';']
               [VAR ident {',' ident}* ';']
               {PROCEDURE ident ';' block ';'}*
               statement

statement  ::= [ident ':=' expression
             |  CALL ident
             |  BEGIN statement {';' statement}* END
             |  IF condition THEN statement
             |  WHILE condition DO statement
             |  ('!' | 'out' | 'write') expression]

condition  ::= ODD expression
             | expression ('='|'#'|'<'|'<='|'>'|'>=') expression

expression ::= ['+' | '-'] term {('+' | '-') term}*
term       ::= factor {('*' | '/') factor}*
factor     ::= ident | number | '(' expression ')'
```

## 最佳实践

### 1. 命名规范
```pascal
VAR counter, total, tempValue;  // 清晰的变量名
```

### 2. 代码缩进
```pascal
BEGIN
  IF x > 0 THEN
  BEGIN
    y := x;
    ! y
  END
END.
```

### 3. 注释（虽然不支持，但建议在文档中说明）
```pascal
// 在外部文档说明程序功能
VAR fibonacci;  // 斐波那契数列的当前值
```

### 4. 错误处理
```pascal
// 避免除零
IF divisor # 0 THEN
  result := dividend / divisor;
```

## 调试技巧

### 1. 输出中间值
```pascal
x := a + b;
! x;         // 检查中间结果
y := x * c;
! y;
```

### 2. 分步执行
```pascal
WHILE x > 0 DO
BEGIN
  ! x;       // 显示循环变量
  x := x - 1
END;
```

### 3. 过程测试
```pascal
PROCEDURE test;
BEGIN
  ! 999      // 标记：过程被调用
END;
```

## 下一步

- 运行 [samples/](../samples/) 中的示例程序
- 阅读 [编译流程](compilation-pipeline.md)
- 了解 [异常处理机制](exception-handling.md)
- 查看 [性能优化](performance.md)
