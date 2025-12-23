#include "symbol_table.h"

namespace pl0 {

using namespace peg::udl;

void SymbolTableBuilder::build_on_ast(const std::shared_ptr<AstPL0> ast,
                                     std::shared_ptr<SymbolScope> scope) {
  switch (ast->tag) {
    case "block"_:
      block(ast, scope);
      break;
    case "assignment"_:
      assignment(ast, scope);
      break;
    case "call"_:
      call(ast, scope);
      break;
    case "ident"_:
      ident(ast, scope);
      break;
    default:
      for (auto node : ast->nodes) {
        build_on_ast(node, scope);
      }
      break;
  }
}

void SymbolTableBuilder::block(const std::shared_ptr<AstPL0> ast,
                               std::shared_ptr<SymbolScope> outer) {
  auto scope = std::make_shared<SymbolScope>(outer);
  const auto& nodes = ast->nodes;
  constants(nodes[0], scope);
  variables(nodes[1], scope);
  procedures(nodes[2], scope);
  build_on_ast(nodes[3], scope);
  ast->scope = scope;
}

void SymbolTableBuilder::constants(const std::shared_ptr<AstPL0> ast,
                                   std::shared_ptr<SymbolScope> scope) {
  const auto& nodes = ast->nodes;
  for (auto i = 0u; i < nodes.size(); i += 2) {
    auto ident = nodes[i + 0]->token;
    if (scope->has_symbol(ident)) {
      throw_runtime_error(
          nodes[i], "'" + std::string(ident) + "' is already defined...");
    }
    auto number = nodes[i + 1]->token_to_number<int>();
    scope->constants.emplace(ident, number);
  }
}

void SymbolTableBuilder::variables(const std::shared_ptr<AstPL0> ast,
                                   std::shared_ptr<SymbolScope> scope) {
  const auto& nodes = ast->nodes;
  for (auto i = 0u; i < nodes.size(); i += 1) {
    auto ident = nodes[i]->token;
    if (scope->has_symbol(ident)) {
      throw_runtime_error(
          nodes[i], "'" + std::string(ident) + "' is already defined...");
    }
    scope->variables.emplace(ident);
  }
}

void SymbolTableBuilder::procedures(const std::shared_ptr<AstPL0> ast,
                                    std::shared_ptr<SymbolScope> scope) {
  const auto& nodes = ast->nodes;
  for (auto i = 0u; i < nodes.size(); i += 2) {
    auto ident = nodes[i + 0]->token;
    auto block = nodes[i + 1];
    scope->procedures[ident] = block;
    build_on_ast(block, scope);
  }
}

void SymbolTableBuilder::assignment(const std::shared_ptr<AstPL0> ast,
                                    std::shared_ptr<SymbolScope> scope) {
  auto ident = ast->nodes[0]->token;
  if (scope->has_constant(ident)) {
    throw_runtime_error(ast->nodes[0], "cannot modify constant value '" +
                                           std::string(ident) + "'...");
  } else if (!scope->has_variable(ident)) {
    throw_runtime_error(ast->nodes[0],
                        "undefined variable '" + std::string(ident) + "'...");
  }

  build_on_ast(ast->nodes[1], scope);

  if (!scope->has_symbol(ident, false)) {
    scope->free_variables.emplace(ident);
  }
}

void SymbolTableBuilder::call(const std::shared_ptr<AstPL0> ast,
                              std::shared_ptr<SymbolScope> scope) {
  auto ident = ast->nodes[0]->token;
  if (!scope->has_procedure(ident)) {
    throw_runtime_error(
        ast->nodes[0], "undefined procedure '" + std::string(ident) + "'...");
  }

  auto block = scope->get_procedure(ident);
  if (block->scope) {
    for (const auto& free : block->scope->free_variables) {
      if (!scope->has_symbol(free, false)) {
        scope->free_variables.emplace(free);
      }
    }
  }
}

void SymbolTableBuilder::ident(const std::shared_ptr<AstPL0> ast,
                               std::shared_ptr<SymbolScope> scope) {
  auto ident = ast->token;
  if (!scope->has_symbol(ident)) {
    throw_runtime_error(ast,
                        "undefined variable '" + std::string(ident) + "'...");
  }

  if (!scope->has_symbol(ident, false)) {
    scope->free_variables.emplace(ident);
  }
}

}  // namespace pl0
