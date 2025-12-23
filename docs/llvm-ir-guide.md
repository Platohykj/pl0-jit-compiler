# LLVM IR 详解

本文档详细解释 PL/0 JIT 编译器如何生成 LLVM 中间表示 (IR)，以及如何阅读和理解生成的 IR 代码。

## 什么是 LLVM IR

LLVM IR (Intermediate Representation) 是：
- **中间语言**: 介于源码和机器码之间
- **类汇编**: 类似汇编但更高级和平台无关
- **强类型**: 每个值都有明确的类型
- **SSA 形式**: 静态单赋值，每个变量只赋值一次
- **可优化**: LLVM 优化器的工作对象

## 查看生成的 IR

### 方法 1: 修改代码查看

编辑 `src/jit_compiler.cc`:

```cpp
void JITCompiler::run(const std::shared_ptr<AstPL0> ast) {
  JITCompiler jit;
  jit.compile(ast);
  jit.dump();  // 取消注释这行
  jit.exec();
}
```

重新编译并运行：
```bash
make
./pl0 samples/square.pas > output.ll
```

### 方法 2: 查看示例文件

项目包含手写的 IR 示例：
```bash
cat divide-by-zero-by-hand.ll
```

## IR 基础语法

### 类型系统

```llvm
i32         ; 32位整数
i64         ; 64位整数
i8          ; 8位整数 (字节)
i8*         ; 指向i8的指针
ptr         ; 通用指针类型 (LLVM 15+)
void        ; 无返回值类型
{i8*, i32}  ; 结构体类型
```

### 变量和值

```llvm
%x          ; 局部变量 (SSA 寄存器)
%0, %1      ; 临时值 (编译器生成)
@out        ; 全局符号 (函数、全局变量)
```

### 基本指令

#### 1. 内存操作

```llvm
; 分配栈内存
%x = alloca i32, align 4

; 存储值
store i32 10, i32* %x, align 4

; 加载值
%0 = load i32, i32* %x, align 4
```

#### 2. 算术运算

```llvm
%sum = add i32 %a, %b      ; 加法
%diff = sub i32 %a, %b     ; 减法
%prod = mul i32 %a, %b     ; 乘法
%quot = sdiv i32 %a, %b    ; 有符号除法
%neg = sub i32 0, %x       ; 取负
```

#### 3. 比较运算

```llvm
%cmp = icmp eq i32 %a, %b   ; 等于
%cmp = icmp ne i32 %a, %b   ; 不等于
%cmp = icmp slt i32 %a, %b  ; 小于 (有符号)
%cmp = icmp sle i32 %a, %b  ; 小于等于
%cmp = icmp sgt i32 %a, %b  ; 大于
%cmp = icmp sge i32 %a, %b  ; 大于等于
```

#### 4. 控制流

```llvm
; 无条件跳转
br label %block

; 条件跳转
br i1 %cond, label %true_block, label %false_block

; 返回
ret void
ret i32 %value
```

#### 5. 函数调用

```llvm
call void @out(i32 42)
%result = call i32 @add(i32 %a, i32 %b)
```

## PL/0 到 IR 的映射

### 变量声明

**PL/0**:
```pascal
VAR x, y;
```

**IR**:
```llvm
%x = alloca i32, align 4
%y = alloca i32, align 4
```

### 常量

**PL/0**:
```pascal
CONST pi = 3;
```

**IR**:
```llvm
%pi = alloca i32, align 4
store i32 3, i32* %pi, align 4
```

### 赋值语句

**PL/0**:
```pascal
x := 10;
```

**IR**:
```llvm
store i32 10, i32* %x, align 4
```

### 表达式

**PL/0**:
```pascal
result := x + y * 2;
```

**IR**:
```llvm
%0 = load i32, i32* %x
%1 = load i32, i32* %y
%2 = mul i32 %1, 2
%3 = add i32 %0, %2
store i32 %3, i32* %result
```

### IF 语句

**PL/0**:
```pascal
IF x > 10 THEN
  ! x;
```

**IR**:
```llvm
%0 = load i32, i32* %x
%cmp = icmp sgt i32 %0, 10
br i1 %cmp, label %if.then, label %if.end

if.then:
  %1 = load i32, i32* %x
  call void @out(i32 %1)
  br label %if.end

if.end:
  ; 继续...
```

### WHILE 循环

**PL/0**:
```pascal
WHILE x > 0 DO
  x := x - 1;
```

**IR**:
```llvm
br label %while.cond

while.cond:
  %0 = load i32, i32* %x
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %while.body, label %while.end

while.body:
  %1 = load i32, i32* %x
  %2 = sub i32 %1, 1
  store i32 %2, i32* %x
  br label %while.cond

while.end:
  ; 继续...
```

### 过程定义和调用

**PL/0**:
```pascal
VAR x;

PROCEDURE inc;
BEGIN
  x := x + 1
END;

BEGIN
  x := 0;
  CALL inc
END.
```

**IR**:
```llvm
; 过程（闭包变量作为参数）
define void @inc(i32* %x) {
entry:
  %0 = load i32, i32* %x
  %1 = add i32 %0, 1
  store i32 %1, i32* %x
  ret void
}

; 主程序
define void @__pl0_start() {
entry:
  %x = alloca i32, align 4
  store i32 0, i32* %x
  call void @inc(i32* %x)  ; 传递变量地址
  ret void
}
```

## 除零检查实现

这是 PL/0 编译器的重要特性，详见 [divide-by-zero-by-hand.ll](../divide-by-zero-by-hand.ll)。

**PL/0**:
```pascal
result := 100 / x;
```

**IR (简化)**:
```llvm
%divisor = load i32, i32* %x

; 检查是否为零
%is_zero = icmp eq i32 %divisor, 0
br i1 %is_zero, label %zdiv.zero, label %zdiv.nonzero

zdiv.zero:
  ; 抛出异常
  %eh = call i8* @__cxa_allocate_exception(i64 8)
  ; ... 设置异常消息
  call void @__cxa_throw(i8* %eh, i8* @typeinfo, i8* null)
  unreachable

zdiv.nonzero:
  ; 正常除法
  %result = sdiv i32 100, %divisor
  store i32 %result, i32* %result_var
  br label %continue

continue:
  ; ...
```

## 异常处理机制

### 异常抛出

```llvm
; 1. 分配异常对象
%eh = call i8* @__cxa_allocate_exception(i64 8)

; 2. 设置异常消息
%payload = bitcast i8* %eh to i8**
store i8* @error_msg, i8** %payload

; 3. 抛出异常
call void @__cxa_throw(i8* %eh, i8* @typeinfo, i8* null)
unreachable
```

### 异常捕获

```llvm
define void @main() personality i8* @__gxx_personality_v0 {
entry:
  ; 使用 invoke 而非 call（可能抛异常）
  invoke void @__pl0_start()
          to label %normal unwind label %exception

exception:
  ; landing pad - 异常着陆点
  %exc = landingpad { i8*, i32 }
          catch i8* @typeinfo
  
  %exc_ptr = extractvalue { i8*, i32 } %exc, 0
  %exc_sel = extractvalue { i8*, i32 } %exc, 1
  
  ; 检查异常类型
  %tid = call i32 @llvm.eh.typeid.for(i8* @typeinfo)
  %matches = icmp eq i32 %exc_sel, %tid
  br i1 %matches, label %catch, label %unexpected

catch:
  ; 开始处理异常
  %msg = call i8* @__cxa_begin_catch(i8* %exc_ptr)
  call i32 @puts(i8* %msg)
  call void @__cxa_end_catch()
  br label %end

unexpected:
  ; 未预期的异常
  call i8* @__cxa_begin_catch(i8* %exc_ptr)
  call i32 @puts(i8* @unknown_error)
  call void @__cxa_end_catch()
  br label %end

normal:
  br label %end

end:
  ret void
}
```

## 完整示例分析

### 源码: square.pas

```pascal
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
```

### 生成的 IR (简化)

```llvm
; 运行时库函数
declare i32 @printf(i8*, ...)

; 输出函数
@.printf.fmt = private constant [4 x i8] c"%d\0A\00"

define void @out(i32 %val) {
entry:
  %fmt = getelementptr [4 x i8], [4 x i8]* @.printf.fmt, i32 0, i32 0
  call i32 (i8*, ...) @printf(i8* %fmt, i32 %val)
  ret void
}

; 过程 square (接收外部变量作为参数)
define void @square(i32* %x, i32* %squ) {
entry:
  %0 = load i32, i32* %x
  %1 = mul i32 %0, %0
  store i32 %1, i32* %squ
  ret void
}

; 主程序
define void @__pl0_start() {
entry:
  %x = alloca i32, align 4
  %squ = alloca i32, align 4
  
  ; x := 1
  store i32 1, i32* %x
  br label %while.cond

while.cond:
  ; 条件: x <= 10
  %0 = load i32, i32* %x
  %cmp = icmp sle i32 %0, 10
  br i1 %cmp, label %while.body, label %while.end

while.body:
  ; CALL square
  call void @square(i32* %x, i32* %squ)
  
  ; ! squ
  %1 = load i32, i32* %squ
  call void @out(i32 %1)
  
  ; x := x + 1
  %2 = load i32, i32* %x
  %3 = add i32 %2, 1
  store i32 %3, i32* %x
  br label %while.cond

while.end:
  ret void
}

; main 函数（带异常处理）
define void @main() personality i8* @__gxx_personality_v0 {
entry:
  invoke void @__pl0_start()
          to label %end unwind label %lpad

lpad:
  ; ... 异常处理代码 ...
  br label %end

end:
  ret void
}
```

## IR 优化示例

### 优化前

```llvm
%x = alloca i32
store i32 10, i32* %x
%0 = load i32, i32* %x
%1 = add i32 %0, 5
%result = alloca i32
store i32 %1, i32* %result
```

### 优化后 (常量传播 + 死代码消除)

```llvm
%result = alloca i32
store i32 15, i32* %result  ; 10+5 在编译时计算
```

## 常见 IR 模式

### 1. 函数序言

```llvm
define void @function() {
entry:
  ; 分配局部变量
  %var1 = alloca i32
  %var2 = alloca i32
  ; ...
}
```

### 2. 基本块命名

```llvm
entry:           ; 入口块
  br label %body

body:            ; 主体
  br i1 %cond, label %then, label %else

then:            ; 条件为真
  br label %end

else:            ; 条件为假
  br label %end

end:             ; 汇合点
  ret void
```

### 3. PHI 节点 (SSA)

```llvm
br i1 %cond, label %then, label %else

then:
  %x1 = add i32 %a, 1
  br label %merge

else:
  %x2 = add i32 %a, 2
  br label %merge

merge:
  ; x 的值取决于来自哪个分支
  %x = phi i32 [ %x1, %then ], [ %x2, %else ]
  ret i32 %x
```

## 调试 IR

### 1. 验证 IR

```bash
llvm-as < output.ll | llvm-dis
```

### 2. 优化 IR

```bash
opt -O3 output.ll -S -o optimized.ll
```

### 3. 查看汇编

```bash
llc output.ll -o output.s
cat output.s
```

### 4. 执行 IR

```bash
lli output.ll
```

## IR 与汇编对比

### IR (平台无关)
```llvm
%sum = add i32 %a, %b
ret i32 %sum
```

### x86-64 汇编
```asm
addl %esi, %edi
movl %edi, %eax
retq
```

### ARM 汇编
```asm
add r0, r0, r1
bx lr
```

## 学习资源

### 官方文档
- [LLVM Language Reference](https://llvm.org/docs/LangRef.html)
- [LLVM Programmer's Manual](https://llvm.org/docs/ProgrammersManual.html)

### 工具
- `opt` - LLVM 优化器
- `llc` - LLVM 编译器
- `lli` - LLVM 解释器
- `llvm-dis` - 反汇编工具

### 实践
1. 修改示例程序
2. 查看生成的 IR
3. 手动编写简单 IR
4. 使用 `lli` 执行

## 下一步

- 阅读 [异常处理机制](exception-handling.md)
- 查看 [性能优化](performance.md)
- 学习 [JIT 编译器 API](api/jit-compiler.md)
- 实验 [divide-by-zero-by-hand.ll](../divide-by-zero-by-hand.ll)
