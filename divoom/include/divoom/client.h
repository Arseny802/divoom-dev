#pragma once
#include "common/common.hpp"
#include <memory>
#include <string>

#include "iclient.h"

namespace divoomdev::divoom {

class client : public iclient {
 public:
  client(std::string host = common::DIVOOMDEV_HOST);
  ~client();

  common::device_list get_devices();
  common::clock_list get_device_clockes(int device_id);
  bool do_reboot(const std::string& device_ip);
  bool set_brightness(const std::string& device_ip, int brightness = 100);
  bool set_mirror(const std::string& device_ip, int mode = 0);
  bool set_time_format(const std::string& device_ip, int mode = 1);
  bool set_clock_id(const std::string& device_ip, int clock_id);

 protected:
  // template<typename T>
  // using HandlerTypePtr = handler<std::shared_ptr<handler<T>>>;

  template<typename HandlerT>
  bool execute(const std::shared_ptr<HandlerT>& handler, std::string host);

  std::string host_;
};

}  // namespace divoomdev::divoom
