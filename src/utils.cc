#include "utils.h"
#include <sstream>

namespace pl0 {

std::string format_error_message(const std::string& path, size_t ln, size_t col,
                                 const std::string& msg) {
  std::stringstream ss;
  ss << path << ":" << ln << ":" << col << ": " << msg << std::endl;
  return ss.str();
}

}  // namespace pl0
