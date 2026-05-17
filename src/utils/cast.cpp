#include "cast.h"

#include <limits>
#include <stdexcept>
#include <cmath>

inline uint64_t safe_double_to_uint64(double d) {
    if (d > std::numeric_limits<uint64_t>::max() || d < 0) throw std::overflow_error("Overflow");
    if (!std::isfinite(d)) throw std::invalid_argument("Not a number");
    return static_cast<uint64_t>(std::floor(d));
}

