#pragma once

#include <stddef.h>

namespace kstring {

[[nodiscard]] int compare(const char *lhs, const char *rhs) noexcept;
[[nodiscard]] bool startsWith(const char *str, const char *prefix) noexcept;

} // namespace kstring
