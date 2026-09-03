#pragma once
#include "common/common.hpp"
#include <memory>
#include <string>

#include "clock_config.h"
#include "iclient.h"
#include "user_info.h"

namespace divoomdev::divoom {

class client : public iclient {
 public:
  client(std::string host = common::DIVOOMDEV_HOST);
  ~client();

  user_info login(std::string email, std::string password);

  common::device_list get_devices();
  common::clock_list get_device_clockes(int device_id);
  common::font_list get_font_list();

  bool do_reboot(const std::string& device_ip);
  bool do_turn_screen(const std::string& device_ip, int turn_on = 1);
  bool get_all_conf(const std::string& device_ip);
  clock_config get_clock_config(int device_id, int clock_id, const user_info& user);
  bool get_clock_info(const std::string& device_ip);
  bool get_index(const std::string& device_ip);
  bool set_brightness(const std::string& device_ip, int brightness = 100);
  bool set_high_light(const std::string& device_ip, int mode = 0);
  bool set_mirror(const std::string& device_ip, int mode = 0);
  bool set_time_format(const std::string& device_ip, int mode = 1);
  bool set_clock_id(const std::string& device_ip, int clock_id);
  bool set_clock_config(int device_id, int clock_id, const user_info& user, const clock_config& clock_config);

  bool patch_clock_info(const std::string& device_ip, common::display_rendered_list patch_list);

  bool cusom_control_enter(const std::string& device_ip,
                           common::display_rendered_list display_list,
                           std::string backgroud_image_addr,
                           int backgroud_image_local_flag = 1);
  bool cusom_control_exit(const std::string& device_ip);
  bool cusom_control_update_text(const std::string& device_ip, common::display_list display_list);

 protected:
  template<typename HandlerT>
  bool execute(const std::shared_ptr<HandlerT>& handler, std::string host);

  std::string host_;
};

}  // namespace divoomdev::divoom
