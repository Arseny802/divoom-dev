#include "login.h"
#include <mbedtls/md.h>

namespace divoomdev::divoom::handlers {

login::login(std::string email, std::string password): handler(RequestType::POST) {
  request_.Command = "UserLogin";
  request_.Email = std::move(email);
  request_.Password = compute_md5(password);
  log()->debug("Email {}, Password {}", request_.Email, request_.Password);
}
login::~login() = default;

std::string login::get_path(const std::string_view host) const noexcept {
  return format("https://{}:443/UserLogin", host);
}

bool login::handle(const std::string& json_str) {
  result_ = std::nullopt;
  log()->info(json_str);
  auto json = parse_json(json_str);
  if (!json) {
    return false;
  }

  result_ = ResultType();
  result_->UserId = json->at("UserId");
  result_->Token = json->at("Token");

  return true;
}

std::string login::compute_md5(const std::string& input) {
  mbedtls_md_context_t ctx;
  unsigned char hash[16];

  mbedtls_md_init(&ctx);
  if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_MD5), 0) != 0) {
    mbedtls_md_free(&ctx);
    return "";
  }

  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);

  std::ostringstream oss;
  for (int i = 0; i < 16; ++i) {
    oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return oss.str();
}

}  // namespace divoomdev::divoom::handlers
