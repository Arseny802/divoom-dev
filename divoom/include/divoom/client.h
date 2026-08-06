#pragma once
#include "common/common.hpp"
#include <memory>
#include <string>
#include <vector>

#include "handler.h"
#include "iclient.h"

namespace divoomdev::divoom {

class client : public iclient {
 public:
  client(std::string host = common::DIVOOMDEV_HOST);
  ~client();

  std::vector<common::device> get_devices();
  bool do_reboot(const std::string& device_ip);
  bool set_brightness(const std::string& device_ip, int brightness = 100);
  bool set_mirror(const std::string& device_ip, int mode = 0);
  bool set_time_format(const std::string& device_ip, int mode = 1);

 protected:
  // template<typename T>
  // using HandlerTypePtr = handler<std::shared_ptr<handler<T>>>;

  template<typename T1, typename T2>
  bool execute(const std::shared_ptr<handler<T1, T2>>& handler, std::string host);

  std::string host_;
};

}  // namespace divoomdev::divoom
