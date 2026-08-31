#include "sf/game/weapon_wheel.hpp"

#include <algorithm>
#include <cmath>

namespace sf::game {

namespace {

constexpr float hold_threshold_seconds = 0.22f;
constexpr float slow_motion_time_scale = 0.20f;
constexpr float pi = 3.14159265358979323846f;

} // namespace

struct WeaponWheel::Impl {
  std::vector<WeaponWheelItem> inventory;
  float hold_timer{0.0f};
  bool is_open{false};
  int hovered_index{-1};
  std::optional<std::uint32_t> pending_selection;

  void updateHold(bool button_held, float stick_x, float stick_y, float dt) {
    if (button_held) {
      hold_timer += dt;
      if (hold_timer >= hold_threshold_seconds && !inventory.empty()) {
        is_open = true;

        const float mag_sq = stick_x * stick_x + stick_y * stick_y;
        if (mag_sq > 0.15f * 0.15f) {
          // Compute angle around circle in range [0, 2pi)
          float angle = std::atan2(-stick_y, stick_x); // Top = positive Y in standard coords
          if (angle < 0.0f) angle += 2.0f * pi;

          const std::size_t count = inventory.size();
          const float slice_angle = (2.0f * pi) / static_cast<float>(count);
          int idx = static_cast<int>(std::floor((angle + slice_angle * 0.5f) / slice_angle)) % count;
          hovered_index = idx;
        }
      }
    } else {
      if (is_open) {
        if (hovered_index >= 0 && static_cast<std::size_t>(hovered_index) < inventory.size()) {
          pending_selection = inventory[static_cast<std::size_t>(hovered_index)].weapon_id;
        }
        is_open = false;
      }
      hold_timer = 0.0f;
      hovered_index = -1;
    }
  }
};

WeaponWheel::WeaponWheel() : impl_(new Impl()) {}
WeaponWheel::~WeaponWheel() { delete impl_; }

void WeaponWheel::updateInventory(std::vector<WeaponWheelItem> items) {
  impl_->inventory = std::move(items);
}

void WeaponWheel::handleHoldInput(bool button_held, float stick_x, float stick_y, float delta_time) {
  impl_->updateHold(button_held, stick_x, stick_y, delta_time);
}

bool WeaponWheel::isOpen() const noexcept {
  return impl_->is_open;
}

float WeaponWheel::timeScale() const noexcept {
  return impl_->is_open ? slow_motion_time_scale : 1.0f;
}

std::optional<std::uint32_t> WeaponWheel::consumeSelectedWeaponId() noexcept {
  auto selected = impl_->pending_selection;
  impl_->pending_selection = std::nullopt;
  return selected;
}

int WeaponWheel::hoveredIndex() const noexcept {
  return impl_->hovered_index;
}

const std::vector<WeaponWheelItem> &WeaponWheel::items() const noexcept {
  return impl_->inventory;
}

} // namespace sf::game
