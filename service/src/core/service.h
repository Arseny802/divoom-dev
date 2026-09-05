#pragma once
#include <chrono>
#include <memory>

namespace divoomdev::ics {
class event_manager;
}

namespace divoomdev::storage {
class i_settings_storage;
}

namespace divoomdev::service::core {

/// Главный цикл сервиса: обрабатывает события календаря и обновляет устройства.
class service {
 public:
  using storage_ptr = std::unique_ptr<storage::i_settings_storage>;

  explicit service(storage_ptr storage);
  ~service();

  // Запускает главный цикл (бесконечный).
  void run();

  /// Ставит главный цикл на паузу.
  void pause();

  /// Возобновляет работу после паузы.
  void resume();

  /// Останавливает главный цикл.
  void stop();

  /// Устанавливает интервал обновления (по умолчанию 5 минут).
  void set_update_interval(std::chrono::minutes interval);

  storage::i_settings_storage* get_storage();

 private:
  void process_cycle();

  storage_ptr storage_;
  std::unique_ptr<class device_updater> device_updater_;
  std::unique_ptr<class event_formatter> event_formatter_;
  std::unique_ptr<ics::event_manager> calendar_manager_;
  std::chrono::minutes update_interval_{5};
};

}  // namespace divoomdev::service::core
