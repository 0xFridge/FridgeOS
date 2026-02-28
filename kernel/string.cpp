#include "string.h"

namespace kstring {

constexpr char EMPTY_STRING[] = "";

int compare(const char *lhs, const char *rhs) noexcept {
  const char *left = (lhs == nullptr) ? EMPTY_STRING : lhs;
  const char *right = (rhs == nullptr) ? EMPTY_STRING : rhs;

  for (size_t index = 0u;; ++index) {
    const char l = left[index];
    const char r = right[index];
    if (l == r) {
      if (l == '\0') {
        return 0;
      }
      continue;
    }

    return (l < r) ? -1 : 1;
  }
}

bool startsWith(const char *str, const char *prefix) noexcept {
  const char *left = (str == nullptr) ? EMPTY_STRING : str;
  const char *right = (prefix == nullptr) ? EMPTY_STRING : prefix;

  for (size_t index = 0u;; ++index) {
    const char l = left[index];
    const char r = right[index];

    if (r == '\0') {
      return true;
    }

    if (l == '\0' || l != r) {
      return false;
    }
  }
}

} // namespace kstring
