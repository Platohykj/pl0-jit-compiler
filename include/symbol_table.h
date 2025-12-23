#ifndef PL0_SYMBOL_TABLE_H
#define PL0_SYMBOL_TABLE_H

#include "ast.h"
#include <memory>

namespace pl0 {

// Symbol table builder - constructs symbol tables during semantic analysis
class SymbolTableBuilder {
 public:
  static void build_on_ast(const std::shared_ptr<AstPL0> ast,
                          std::shared_ptr<SymbolScope> scope = nullptr);

 private:
  static void block(const std::shared_ptr<AstPL0> ast,
                   std::shared_ptr<SymbolScope> outer);
  static void constants(const std::shared_ptr<AstPL0> ast,
                       std::shared_ptr<SymbolScope> scope);
  static void variables(const std::shared_ptr<AstPL0> ast,
                       std::shared_ptr<SymbolScope> scope);
  static void procedures(const std::shared_ptr<AstPL0> ast,
                        std::shared_ptr<SymbolScope> scope);
  static void assignment(const std::shared_ptr<AstPL0> ast,
                        std::shared_ptr<SymbolScope> scope);
  static void call(const std::shared_ptr<AstPL0> ast,
                  std::shared_ptr<SymbolScope> scope);
  static void ident(const std::shared_ptr<AstPL0> ast,
                   std::shared_ptr<SymbolScope> scope);
};

}  // namespace pl0

#endif  // PL0_SYMBOL_TABLE_H
