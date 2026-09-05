
namespace divoomdev::divoom {

hare::hlogger_ptr get_logger(const std::string& path) {
  if (!path.empty()) {
    hare::config_ptr cfg = std::make_unique<hare::config_default>(PROJECT_NAME, MODULE_NAME);
    cfg->set_log_path(path);
    hare::register_logger(std::move(cfg));
  }
  return log();
}

}  // namespace divoomdev::divoom