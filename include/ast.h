#ifndef PL0_AST_H
#define PL0_AST_H

#include <peglib.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>

namespace pl0 {

// Forward declarations
struct SymbolScope;

// Annotation for AST nodes
struct Annotation {
  std::shared_ptr<SymbolScope> scope;
};

// PL/0 AST type
typedef peg::AstBase<Annotation> AstPL0;

// Symbol scope for managing constants, variables, and procedures
struct SymbolScope {
  SymbolScope(std::shared_ptr<SymbolScope> outer) : outer(outer) {}

  bool has_symbol(std::string_view ident, bool extend = true) const {
    auto ret = constants.count(ident) || variables.count(ident);
    return ret ? true : (extend && outer ? outer->has_symbol(ident) : false);
  }

  bool has_constant(std::string_view ident) const {
    return constants.count(ident)
               ? true
               : (outer ? outer->has_constant(ident) : false);
  }

  bool has_variable(std::string_view ident) const {
    return variables.count(ident)
               ? true
               : (outer ? outer->has_variable(ident) : false);
  }

  bool has_procedure(std::string_view ident) const {
    return procedures.count(ident)
               ? true
               : (outer ? outer->has_procedure(ident) : false);
  }

  std::shared_ptr<AstPL0> get_procedure(std::string_view ident) const {
    auto it = procedures.find(ident);
    return it != procedures.end() ? it->second : outer->get_procedure(ident);
  }

  std::map<std::string_view, int> constants;
  std::set<std::string_view> variables;
  std::map<std::string_view, std::shared_ptr<AstPL0>> procedures;
  std::set<std::string_view> free_variables;

 private:
  std::shared_ptr<SymbolScope> outer;
};

// Helper function to get closest scope from AST node
std::shared_ptr<SymbolScope> get_closest_scope(std::shared_ptr<AstPL0> ast);

// Helper function to throw runtime error with location info
void throw_runtime_error(const std::shared_ptr<AstPL0> node,
                        const std::string& msg);

}  // namespace pl0

#endif  // PL0_AST_H
