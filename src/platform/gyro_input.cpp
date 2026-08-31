#include "sf/platform/gyro_input.hpp"

#include <SDL.h>
#include <algorithm>
#include <cmath>

namespace sf::platform {

struct GyroController::Impl {
  GyroConfig config;
  float accumulated_yaw{0.0f};
  float accumulated_pitch{0.0f};
  bool sensor_enabled{false};

  void updateController(SDL_GameController *controller, float delta_time_seconds) {
    if (!config.enabled || controller == nullptr) {
      return;
    }

#if SDL_VERSION_ATLEAST(2, 0, 14)
    if (!sensor_enabled) {
      if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO) == SDL_TRUE) {
        SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE);
        sensor_enabled = true;
      }
    }

    if (sensor_enabled) {
      float data[3]{0.0f, 0.0f, 0.0f};
      if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_GYRO, data, 3) == 0) {
        processGyroValues(data[0], data[1], data[2], delta_time_seconds);
      }
    }
#endif
  }

  void processGyroValues(float gx, float gy, float gz, float dt) {
    if (std::abs(gx) < config.deadzone) gx = 0.0f;
    if (std::abs(gy) < config.deadzone) gy = 0.0f;
    if (std::abs(gz) < config.deadzone) gz = 0.0f;

    // Pitch: rotation around X axis, Yaw: rotation around Y/Z axis depending on controller orientation
    float yaw_rate = -gz;
    float pitch_rate = gx;

    if (config.invert_x) yaw_rate = -yaw_rate;
    if (config.invert_y) pitch_rate = -pitch_rate;

    accumulated_yaw += yaw_rate * config.sensitivity_x * dt;
    accumulated_pitch += pitch_rate * config.sensitivity_y * dt;
  }
};

GyroController::GyroController() : impl_(new Impl()) {}
GyroController::~GyroController() { delete impl_; }

void GyroController::setConfig(const GyroConfig &config) {
  impl_->config = config;
}

const GyroConfig &GyroController::config() const noexcept {
  return impl_->config;
}

void GyroController::updateFromController(SDL_GameController *controller, float delta_time_seconds) {
  impl_->updateController(controller, delta_time_seconds);
}

void GyroController::feedSensorData(float gyro_x, float gyro_y, float gyro_z, float delta_time_seconds) {
  if (!impl_->config.enabled) return;
  impl_->processGyroValues(gyro_x, gyro_y, gyro_z, delta_time_seconds);
}

GyroDelta GyroController::consumeDelta() noexcept {
  GyroDelta delta{impl_->accumulated_yaw, impl_->accumulated_pitch};
  impl_->accumulated_yaw = 0.0f;
  impl_->accumulated_pitch = 0.0f;
  return delta;
}

void GyroController::reset() noexcept {
  impl_->accumulated_yaw = 0.0f;
  impl_->accumulated_pitch = 0.0f;
}

GyroController &globalGyroController() {
  static GyroController instance;
  return instance;
}

} // namespace sf::platform
