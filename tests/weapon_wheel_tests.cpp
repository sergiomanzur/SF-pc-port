#include "sf/game/weapon_wheel.hpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

void require(bool condition, std::string_view message) {
  if (!condition) {
    throw std::runtime_error{std::string{message}};
  }
}

void testWeaponWheelHoldAndSelect() {
  using namespace sf::game;
  WeaponWheel wheel;

  std::vector<WeaponWheelItem> inventory{
      {1U, "Silenced 9mm", 15U, 45U, true},
      {2U, ".45", 10U, 30U, false},
      {3U, "PK-102", 30U, 90U, false},
      {4U, "Sniper Rifle", 10U, 20U, false},
      {5U, "Taser", 0U, 0U, false},
  };
  wheel.updateInventory(inventory);

  require(!wheel.isOpen(), "wheel should initially be closed");
  require(wheel.timeScale() == 1.0f, "initial time scale should be 1.0");

  // Quick tap (< 0.22s) should not open the wheel
  wheel.handleHoldInput(true, 0.0f, 0.0f, 0.10f);
  require(!wheel.isOpen(), "short tap opened wheel");
  wheel.handleHoldInput(false, 0.0f, 0.0f, 0.01f);
  require(!wheel.isOpen(), "wheel should be closed after release");

  // Hold > 0.22s opens wheel and slows down time
  wheel.handleHoldInput(true, 0.0f, 0.0f, 0.25f);
  require(wheel.isOpen(), "wheel should be open after hold threshold");
  require(wheel.timeScale() < 1.0f, "time scale should slow down during wheel selection");

  // Tilt stick right (+X)
  wheel.handleHoldInput(true, 1.0f, 0.0f, 0.016f);
  require(wheel.isOpen(), "wheel remains open while held");

  // Release button confirms selection
  wheel.handleHoldInput(false, 1.0f, 0.0f, 0.016f);
  require(!wheel.isOpen(), "wheel closed on release");
  require(wheel.timeScale() == 1.0f, "time scale restored on release");

  const auto selected = wheel.consumeSelectedWeaponId();
  require(selected.has_value(), "a weapon should have been selected");
}

} // namespace

int main() {
  try {
    testWeaponWheelHoldAndSelect();
    std::cout << "weapon_wheel_tests: ok\n";
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "weapon_wheel_tests failed: " << ex.what() << '\n';
    return 1;
  }
}
