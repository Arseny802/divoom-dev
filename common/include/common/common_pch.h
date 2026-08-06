#pragma once
#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <chrono>
#include <cmath>
#include <complex>
#include <coroutine>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <exception>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <ranges>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <stack>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

// std::format OR fmt::format
#if __has_include(<format>)
#  include <format>
#else
#  include <fmt/core.h>
#  include <fmt/format.h>
#  include <fmt/ranges.h>
#endif

/**
 * Format a string using a given format string and arguments.
 *
 * If the `std::format` header is available, this function will use it.
 * Otherwise, it will use the fmt library.
 *
 * @param fmt_str The format string.
 * @param args The arguments to pass to the format function.
 *
 * @return The formatted string.
 */
template<typename... Args>
inline std::string format(const char fmt_str[], Args&&... args) {
#ifdef __cpp_lib_format
  return std::vformat(fmt_str, std::make_format_args(args...));
#else
  return fmt::vformat(fmt_str, fmt::make_format_args(args...));
#endif
}

// external libraries
#include <magic_enum.hpp>
