#pragma once

#include <cstdint>

struct SDL_Window;
union SDL_Event;

namespace sf::platform {

enum class TouchButton : std::uint32_t {
  none = 0,
  dpad_up = 1 << 0,
  dpad_down = 1 << 1,
  dpad_left = 1 << 2,
  dpad_right = 1 << 3,
  triangle = 1 << 4,
  circle = 1 << 5,
  cross = 1 << 6,
  square = 1 << 7,
  l1 = 1 << 8,
  r1 = 1 << 9,
  l2 = 1 << 10,
  r2 = 1 << 11,
  start = 1 << 12,
  select = 1 << 13,
  l3 = 1 << 14,
  r3 = 1 << 15,
};

struct TouchState {
  std::uint16_t buttons_down{0};
  float left_stick_x{0.0f};
  float left_stick_y{0.0f};
  float right_stick_x{0.0f};
  float right_stick_y{0.0f};
};

struct TouchOverlayConfig {
  bool enabled{true};
  float opacity{0.65f};
  float scale{1.0f};
  bool enable_haptics{true};
  bool auto_hide_on_controller{true};
  bool use_analog_sticks{true};
};

class TouchOverlay {
public:
  TouchOverlay();
  ~TouchOverlay();

  void setConfig(const TouchOverlayConfig &config);
  [[nodiscard]] const TouchOverlayConfig &config() const noexcept;

  void setViewport(int width, int height);
  void handleEvent(const SDL_Event &event);
  [[nodiscard]] TouchState sampleState() const noexcept;

  void render();

  void setPhysicalDeviceConnected(bool connected);
  [[nodiscard]] bool isVisible() const noexcept;

private:
  struct Impl;
  Impl *impl_{nullptr};
};

[[nodiscard]] TouchOverlay &globalTouchOverlay();

} // namespace sf::platform
