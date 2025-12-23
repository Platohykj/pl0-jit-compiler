#include "ast.h"
#include "utils.h"
#include <stdexcept>

namespace pl0 {

using namespace peg::udl;

std::shared_ptr<SymbolScope> get_closest_scope(std::shared_ptr<AstPL0> ast) {
  ast = ast->parent.lock();
  while (ast->tag != "block"_) {
    ast = ast->parent.lock();
  }
  return ast->scope;
}

void throw_runtime_error(const std::shared_ptr<AstPL0> node,
                        const std::string& msg) {
  throw std::runtime_error(
      format_error_message(node->path, node->line, node->column, msg));
}

}  // namespace pl0
