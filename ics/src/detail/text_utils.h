#pragma once
#include <string>
#include <tuple>

namespace divoomdev::ics::detail {

/// Убирает обрамляющие пробельные символы.
std::string trim(const std::string& s);

/// Приводит строку к верхнему регистру (ASCII).
std::string to_upper(const std::string& s);

/// Снимает ICS-экранирование: \n, \N, \,, \;, \\.
std::string unescape(const std::string& value);

/// Разворачивает длинные строки по RFC 5545 (line folding) за один проход.
/// Строка, начинающаяся с пробела/таба — продолжение предыдущей логической строки.
std::string unfold(const std::string& content);

/// Разделяет "KEY[;PARAM=..]:VALUE" на {KEY(верхний регистр), params, value}.
std::tuple<std::string, std::string, std::string> split_ics_line(const std::string& line);

}  // namespace divoomdev::ics::detail