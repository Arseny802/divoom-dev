#include "text_utils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace divoomdev::ics::detail {

std::string trim(const std::string& s) {
  auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos)
    return "";
  auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::string to_upper(const std::string& s) {
  std::string r = s;
  std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return std::toupper(c); });
  return r;
}

std::string unescape(const std::string& value) {
  std::string result;
  result.reserve(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '\\' && i + 1 < value.size()) {
      switch (value[i + 1]) {
      case 'n':
      case 'N':
        result += '\n';
        i++;
        break;
      case ',':
      case ';':
      case '\\':
        result += value[i + 1];
        i++;
        break;
      default: result += value[i]; break;
      }
    } else {
      result += value[i];
    }
  }
  return result;
}

std::string unfold(const std::string& content) {
  std::string lines;
  std::istringstream stream(content);
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.empty()) {
      lines += "\n";
      continue;
    }
    if (line[0] == ' ' || line[0] == '\t') {
      // Продолжение предыдущей логической строки: убираем перевод строки и ведущий пробел.
      if (!lines.empty() && lines.back() == '\n')
        lines.pop_back();
      lines += line.substr(1);
    } else {
      // Новая логическая строка.
      if (!lines.empty() && lines.back() != '\n')
        lines += '\n';
      lines += line + "\n";
    }
  }
  return lines;
}

std::tuple<std::string, std::string, std::string> split_ics_line(const std::string& line) {
  std::string key, params, value;

  auto colon_pos = line.find(':');
  if (colon_pos == std::string::npos)
    return {"", "", ""};

  std::string left = line.substr(0, colon_pos);
  value = line.substr(colon_pos + 1);

  auto semi_pos = left.find(';');
  if (semi_pos != std::string::npos) {
    key = left.substr(0, semi_pos);
    params = left.substr(semi_pos + 1);
  } else {
    key = left;
    params = "";
  }

  return {to_upper(key), params, value};
}

}  // namespace divoomdev::ics::detail