#ifndef PL0_UTILS_H
#define PL0_UTILS_H

#include <string>

namespace pl0 {

std::string format_error_message(const std::string& path, size_t ln, size_t col,
                                 const std::string& msg);

}  // namespace pl0

#endif  // PL0_UTILS_H
