#include "divoom/exceptions.h"

namespace divoomdev::divoom {

const char* TokenExpiredException::what() const noexcept {
  return "Token is wrong or expired (mismatched)";
}

const char* WrongUserException::what() const noexcept {
  return "Wrong user";
}

const char* WrongPasswordException::what() const noexcept {
  return "Wrong password";
}

}  // namespace divoomdev::divoom
