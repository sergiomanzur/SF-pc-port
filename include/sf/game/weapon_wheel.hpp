#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace sf::game {

struct WeaponWheelItem {
  std::uint32_t weapon_id{0};
  std::string_view name;
  std::uint32_t current_ammo{0};
  std::uint32_t max_ammo{0};
  bool is_equipped{false};
};

class WeaponWheel {
public:
  WeaponWheel();
  ~WeaponWheel();

  void updateInventory(std::vector<WeaponWheelItem> items);
  void handleHoldInput(bool button_held, float stick_x, float stick_y, float delta_time);

  [[nodiscard]] bool isOpen() const noexcept;
  [[nodiscard]] float timeScale() const noexcept;
  [[nodiscard]] std::optional<std::uint32_t> consumeSelectedWeaponId() noexcept;

  [[nodiscard]] int hoveredIndex() const noexcept;
  [[nodiscard]] const std::vector<WeaponWheelItem> &items() const noexcept;

private:
  struct Impl;
  Impl *impl_{nullptr};
};

} // namespace sf::game
