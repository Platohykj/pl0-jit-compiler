#ifndef PL0_JIT_COMPILER_H
#define PL0_JIT_COMPILER_H

#include "ast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>

namespace pl0 {

// JIT compiler for PL/0 using LLVM
class JITCompiler {
 public:
  // Compile and execute the AST
  static void run(const std::shared_ptr<AstPL0> ast);

 private:
  llvm::LLVMContext context_;
  llvm::IRBuilder<> builder_;
  std::unique_ptr<llvm::Module> module_;
  llvm::GlobalVariable* tyinfo_ = nullptr;

  JITCompiler();

  void compile(const std::shared_ptr<AstPL0> ast);
  void exec();
  void dump();

  // Compilation methods
  void compile_libs();
  void compile_program(const std::shared_ptr<AstPL0> ast);
  void compile_block(const std::shared_ptr<AstPL0> ast);
  void compile_const(const std::shared_ptr<AstPL0> ast);
  void compile_var(const std::shared_ptr<AstPL0> ast);
  void compile_procedure(const std::shared_ptr<AstPL0> ast);
  void compile_statement(const std::shared_ptr<AstPL0> ast);
  void compile_assignment(const std::shared_ptr<AstPL0> ast);
  void compile_call(const std::shared_ptr<AstPL0> ast);
  void compile_statements(const std::shared_ptr<AstPL0> ast);
  void compile_if(const std::shared_ptr<AstPL0> ast);
  void compile_while(const std::shared_ptr<AstPL0> ast);
  void compile_out(const std::shared_ptr<AstPL0> ast);

  // Value compilation methods
  llvm::Value* compile_condition(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_odd(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_compare(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_expression(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_term(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_factor(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_ident(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_number(const std::shared_ptr<AstPL0> ast);

  // Helper methods
  void compile_switch(const std::shared_ptr<AstPL0> ast);
  llvm::Value* compile_switch_value(const std::shared_ptr<AstPL0> ast);
};

}  // namespace pl0

#endif  // PL0_JIT_COMPILER_H
