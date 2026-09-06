#pragma once

namespace divoomdev::divoom {

class TokenExpiredException : public std::exception {
 public:
  const char* what() const noexcept override;
};

class WrongUserException : public std::exception {
 public:
  const char* what() const noexcept override;
};

class WrongPasswordException : public std::exception {
 public:
  const char* what() const noexcept override;
};

}  // namespace divoomdev::divoom
