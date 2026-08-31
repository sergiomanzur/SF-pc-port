#include "sf/platform/touch_overlay.hpp"

#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <vector>

namespace sf::platform {

namespace {

constexpr float pi = 3.14159265358979323846f;

struct TouchElement {
  TouchButton button{TouchButton::none};
  float norm_x{0.0f}; // normalized 0..1
  float norm_y{0.0f}; // normalized 0..1
  float radius{0.08f};
  bool is_stick{false};
  bool is_pressed{false};
};

} // namespace

struct TouchOverlay::Impl {
  TouchOverlayConfig config;
  int screen_width{1280};
  int screen_height{720};
  bool physical_controller_connected{false};

  std::vector<TouchElement> elements;
  std::map<SDL_FingerID, TouchButton> active_finger_buttons;
  std::map<SDL_FingerID, bool> active_finger_sticks;

  TouchState state{};

  Impl() {
    setupDefaultLayout();
  }

  void setupDefaultLayout() {
    elements.clear();

    // D-Pad / Left Stick area
    elements.push_back({TouchButton::dpad_up,    0.14f, 0.68f, 0.05f, false, false});
    elements.push_back({TouchButton::dpad_down,  0.14f, 0.88f, 0.05f, false, false});
    elements.push_back({TouchButton::dpad_left,  0.08f, 0.78f, 0.05f, false, false});
    elements.push_back({TouchButton::dpad_right, 0.20f, 0.78f, 0.05f, false, false});

    // Face Buttons (PSX Triangle, Circle, Cross, Square)
    elements.push_back({TouchButton::triangle, 0.88f, 0.68f, 0.055f, false, false}); // Top
    elements.push_back({TouchButton::circle,   0.94f, 0.78f, 0.055f, false, false}); // Right
    elements.push_back({TouchButton::cross,    0.88f, 0.88f, 0.055f, false, false}); // Bottom
    elements.push_back({TouchButton::square,   0.82f, 0.78f, 0.055f, false, false}); // Left

    // Shoulder Buttons (L1, L2, R1, R2)
    elements.push_back({TouchButton::l1, 0.10f, 0.12f, 0.06f, false, false});
    elements.push_back({TouchButton::l2, 0.10f, 0.26f, 0.06f, false, false});
    elements.push_back({TouchButton::r1, 0.90f, 0.12f, 0.06f, false, false});
    elements.push_back({TouchButton::r2, 0.90f, 0.26f, 0.06f, false, false});

    // System Buttons (Select, Start)
    elements.push_back({TouchButton::select, 0.42f, 0.90f, 0.045f, false, false});
    elements.push_back({TouchButton::start,  0.58f, 0.90f, 0.045f, false, false});
  }

  void handleFingerDown(const SDL_TouchFingerEvent &finger) {
    const float aspect = screen_width > 0 && screen_height > 0
                             ? static_cast<float>(screen_width) / static_cast<float>(screen_height)
                             : 1.777f;

    for (auto &elem : elements) {
      const float dx = (finger.x - elem.norm_x) * aspect;
      const float dy = (finger.y - elem.norm_y);
      const float dist_sq = dx * dx + dy * dy;
      const float r = elem.radius * config.scale;

      if (dist_sq <= r * r) {
        elem.is_pressed = true;
        active_finger_buttons[finger.fingerId] = elem.button;
        state.buttons_down |= static_cast<std::uint16_t>(elem.button);
        break;
      }
    }

    // Left analog stick virtual area
    if (config.use_analog_sticks && finger.x < 0.35f && finger.y > 0.5f) {
      const float center_x = 0.14f;
      const float center_y = 0.78f;
      const float dx = (finger.x - center_x) * aspect;
      const float dy = (finger.y - center_y);
      state.left_stick_x = std::clamp(dx / 0.12f, -1.0f, 1.0f);
      state.left_stick_y = std::clamp(dy / 0.12f, -1.0f, 1.0f);
      active_finger_sticks[finger.fingerId] = true;
    }
  }

  void handleFingerMotion(const SDL_TouchFingerEvent &finger) {
    const float aspect = screen_width > 0 && screen_height > 0
                             ? static_cast<float>(screen_width) / static_cast<float>(screen_height)
                             : 1.777f;

    if (active_finger_sticks.contains(finger.fingerId)) {
      const float center_x = 0.14f;
      const float center_y = 0.78f;
      const float dx = (finger.x - center_x) * aspect;
      const float dy = (finger.y - center_y);
      state.left_stick_x = std::clamp(dx / 0.12f, -1.0f, 1.0f);
      state.left_stick_y = std::clamp(dy / 0.12f, -1.0f, 1.0f);
    }
  }

  void handleFingerUp(const SDL_TouchFingerEvent &finger) {
    if (auto it = active_finger_buttons.find(finger.fingerId); it != active_finger_buttons.end()) {
      state.buttons_down &= ~static_cast<std::uint16_t>(it->second);
      for (auto &elem : elements) {
        if (elem.button == it->second) {
          elem.is_pressed = false;
        }
      }
      active_finger_buttons.erase(it);
    }

    if (active_finger_sticks.contains(finger.fingerId)) {
      state.left_stick_x = 0.0f;
      state.left_stick_y = 0.0f;
      active_finger_sticks.erase(finger.fingerId);
    }
  }
};

TouchOverlay::TouchOverlay() : impl_(new Impl()) {}
TouchOverlay::~TouchOverlay() { delete impl_; }

void TouchOverlay::setConfig(const TouchOverlayConfig &config) {
  impl_->config = config;
}

const TouchOverlayConfig &TouchOverlay::config() const noexcept {
  return impl_->config;
}

void TouchOverlay::setViewport(int width, int height) {
  impl_->screen_width = width;
  impl_->screen_height = height;
}

void TouchOverlay::setPhysicalDeviceConnected(bool connected) {
  impl_->physical_controller_connected = connected;
}

bool TouchOverlay::isVisible() const noexcept {
  if (!impl_->config.enabled) return false;
  if (impl_->config.auto_hide_on_controller && impl_->physical_controller_connected) {
    return false;
  }
  return true;
}

void TouchOverlay::handleEvent(const SDL_Event &event) {
  if (!impl_->config.enabled) return;

  switch (event.type) {
  case SDL_FINGERDOWN:
    impl_->handleFingerDown(event.tfinger);
    break;
  case SDL_FINGERMOTION:
    impl_->handleFingerMotion(event.tfinger);
    break;
  case SDL_FINGERUP:
    impl_->handleFingerUp(event.tfinger);
    break;
  default:
    break;
  }
}

TouchState TouchOverlay::sampleState() const noexcept {
  return impl_->state;
}

void TouchOverlay::render() {
  // Rendering will be invoked during host frame presentation if overlay is visible
}

TouchOverlay &globalTouchOverlay() {
  static TouchOverlay instance;
  return instance;
}

} // namespace sf::platform
