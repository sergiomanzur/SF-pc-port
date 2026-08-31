#pragma once

#include <cstdint>

struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;

namespace sf::platform {

struct GyroConfig {
  bool enabled{false};
  float sensitivity_x{1.0f};
  float sensitivity_y{1.0f};
  bool invert_x{false};
  bool invert_y{false};
  float deadzone{0.02f};
  bool require_aim_button{true}; // Only active when aiming
};

struct GyroDelta {
  float delta_yaw{0.0f};
  float delta_pitch{0.0f};
};

class GyroController {
public:
  GyroController();
  ~GyroController();

  void setConfig(const GyroConfig &config);
  [[nodiscard]] const GyroConfig &config() const noexcept;

  void updateFromController(SDL_GameController *controller, float delta_time_seconds);
  void feedSensorData(float gyro_x, float gyro_y, float gyro_z, float delta_time_seconds);

  [[nodiscard]] GyroDelta consumeDelta() noexcept;
  void reset() noexcept;

private:
  struct Impl;
  Impl *impl_{nullptr};
};

[[nodiscard]] GyroController &globalGyroController();

} // namespace sf::platform
