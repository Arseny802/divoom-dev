#pragma once
#include "common/event.h"
#include "scheduled_event.h"
#include <list>
#include <string>
#include <vector>

namespace divoomdev::ics {

class event_manager {
 public:
  event_manager();
  ~event_manager();

  /// Добавляет URL календаря для загрузки
  void add_calendar_url(const std::string& url);

  /// Добавляет событие вручную
  void add_event(scheduled_event event);

  /// Загружает календари по всем добавленным URL и возвращает события
  /// на ближайшие 48 часов (от текущего момента).
  /// События со статусом CANCELED исключаются.
  common::event_list get_next_48h_events();

  /// Возвращает отфильтрованный список scheduled_event на 48 часов
  scheduled_event_list get_next_48h_scheduled();

 private:
  scheduled_event_list events_;
  std::list<std::string> calendar_urls_;
};

}  // namespace divoomdev::ics
