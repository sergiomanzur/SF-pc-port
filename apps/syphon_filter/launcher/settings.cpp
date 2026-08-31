#include "settings.hpp"

#ifdef _WIN32

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace sf::platform::launcher_settings {
namespace {

constexpr int minimum_resolution_width = 320;
constexpr int minimum_resolution_height = 240;

std::wstring widenAscii(std::string_view text) {
  return {text.begin(), text.end()};
}

std::filesystem::path settingsPath(bool create_directory,
                                   bool *directory_ready = nullptr) {
  std::filesystem::path directory;
  std::array<wchar_t, 32768U> buffer{};
  const auto length = GetEnvironmentVariableW(
      L"LOCALAPPDATA", buffer.data(), static_cast<DWORD>(buffer.size()));
  if (length > 0U && length < buffer.size()) {
    directory = std::filesystem::path{buffer.data()} / L"SyphonFilterPC";
  } else {
    const auto module_length = GetModuleFileNameW(
        nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    directory = module_length > 0U && module_length < buffer.size()
                    ? std::filesystem::path{buffer.data()}.parent_path()
                    : std::filesystem::current_path();
  }
  if (create_directory) {
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    const auto is_directory =
        !error && std::filesystem::is_directory(directory, error);
    if (directory_ready != nullptr) {
      *directory_ready = is_directory && !error;
    }
  } else if (directory_ready != nullptr) {
    *directory_ready = true;
  }
  return directory / L"launcher.ini";
}

int readProfileInteger(const std::filesystem::path &path,
                       const wchar_t *section, const wchar_t *key,
                       int fallback) noexcept {
  return static_cast<int>(GetPrivateProfileIntW(
      section, key, static_cast<UINT>(fallback), path.c_str()));
}

bool writeProfileInteger(const std::filesystem::path &path,
                         const wchar_t *section, const wchar_t *key,
                         int value) {
  const auto text = std::to_wstring(value);
  return WritePrivateProfileStringW(section, key, text.c_str(), path.c_str()) !=
         FALSE;
}

bool writeControllerBindingsFile(const std::filesystem::path &path,
                                 const ControllerButtonBindings &bindings) {
  if (!game::areControllerBindingsValid(bindings)) {
    return false;
  }
  std::wstring section_data;
  for (const auto &metadata : game::controllerActionCatalog()) {
    const auto key = widenAscii(metadata.config_key);
    const auto value =
        std::to_wstring(static_cast<int>(bindings[metadata.action]));
    section_data.append(key);
    section_data.push_back(L'=');
    section_data.append(value);
    section_data.push_back(L'\0');
  }
  section_data.append(L"StickLayout=");
  section_data.append(std::to_wstring(static_cast<int>(bindings.stick_layout)));
  section_data.push_back(L'\0');
  section_data.push_back(L'\0');
  const auto stored =
      WritePrivateProfileSectionW(L"ControllerBindings", section_data.c_str(),
                                  path.c_str()) != FALSE;
  return stored;
}

bool saveGameImagePath(const std::filesystem::path &path,
                       const std::filesystem::path &cue_path) {
  return WritePrivateProfileStringW(L"Game", L"Image", cue_path.c_str(),
                                    path.c_str()) != FALSE;
}

bool profileStringEquals(const std::filesystem::path &path,
                         const wchar_t *section, const wchar_t *key,
                         std::wstring_view expected) {
  std::array<wchar_t, 32768U> buffer{};
  const auto length = GetPrivateProfileStringW(
      section, key, L"", buffer.data(), static_cast<DWORD>(buffer.size()),
      path.c_str());
  return length == expected.size() &&
         std::wstring_view{buffer.data(), length} == expected;
}

bool profileIntegerEquals(const std::filesystem::path &path,
                          const wchar_t *section, const wchar_t *key,
                          int expected) {
  return profileStringEquals(path, section, key, std::to_wstring(expected));
}

bool settingsSnapshotMatches(const std::filesystem::path &path,
                             const GraphicsSettings &graphics,
                             const KeyboardMouseBindings &input,
                             game::GameLanguage language,
                             const std::filesystem::path &cue_path) {
  bool matches = true;
  const auto check = [&matches](bool result) noexcept {
    matches = result && matches;
  };
  check(profileIntegerEquals(path, L"Graphics", L"Width", graphics.width));
  check(profileIntegerEquals(path, L"Graphics", L"Height", graphics.height));
  check(profileIntegerEquals(path, L"Graphics", L"MSAA",
                             graphics.smaa ? 0 : graphics.msaa_samples));
  check(profileIntegerEquals(path, L"Graphics", L"Bilinear",
                             graphics.bilinear_filtering ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"Trilinear",
                             graphics.trilinear_filtering ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"Anisotropic",
                             graphics.anisotropic_filtering ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"SMAA",
                             graphics.smaa ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"VolumetricEffects",
                             graphics.volumetric_effects ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"MissionSkyboxes",
                             graphics.mission_skyboxes ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"VSync",
                             graphics.vsync ? 1 : 0));
  check(profileIntegerEquals(path, L"Graphics", L"FrameLimit",
                             static_cast<int>(graphics.frame_limit)));
  check(profileIntegerEquals(path, L"Graphics", L"Fullscreen",
                             graphics.fullscreen ? 1 : 0));
  check(profileIntegerEquals(
      path, L"Graphics", L"Aspect",
      graphics.aspect_ratio == AspectRatioMode::adaptive ? 0 : 1));
  check(profileIntegerEquals(path, L"Controller", L"Protocol",
                             static_cast<int>(graphics.controller_protocol)));
  check(profileIntegerEquals(path, L"Controller", L"Vibration",
                             graphics.controller_vibration ? 1 : 0));
  check(profileIntegerEquals(
      path, L"Game", L"Locale",
      language == game::GameLanguage::russian_vit ? 1 : 0));
  for (std::size_t index = 0U; index < keyboard_mouse_action_count; ++index) {
    const auto action = static_cast<KeyboardMouseAction>(index);
    const auto key = widenAscii(keyboardMouseActionConfigKey(action));
    check(profileIntegerEquals(path, L"KeyboardMouse", key.c_str(),
                               static_cast<int>(input[action])));
  }
  for (const auto &metadata : game::controllerActionCatalog()) {
    const auto key = widenAscii(metadata.config_key);
    check(profileIntegerEquals(
        path, L"ControllerBindings", key.c_str(),
        static_cast<int>(graphics.controller_bindings[metadata.action])));
  }
  check(profileIntegerEquals(
      path, L"ControllerBindings", L"StickLayout",
      static_cast<int>(graphics.controller_bindings.stick_layout)));
  check(profileStringEquals(path, L"Game", L"Image", cue_path.native()));
  return matches;
}

} // namespace

std::filesystem::path executableDirectory() {
  std::array<wchar_t, 32768U> buffer{};
  const auto length = GetModuleFileNameW(nullptr, buffer.data(),
                                         static_cast<DWORD>(buffer.size()));
  if (length == 0U || length >= buffer.size()) {
    return std::filesystem::current_path();
  }
  return std::filesystem::path{buffer.data()}.parent_path();
}

std::filesystem::path loadGameImagePath() {
  const auto path = settingsPath(false);
  std::array<wchar_t, 32768U> buffer{};
  const auto length =
      GetPrivateProfileStringW(L"Game", L"Image", L"", buffer.data(),
                               static_cast<DWORD>(buffer.size()), path.c_str());
  if (length == 0U || length >= buffer.size() - 1U) {
    return {};
  }
  return std::filesystem::path{buffer.data()};
}

void loadSettingsFile(GraphicsSettings &graphics, KeyboardMouseBindings &input,
                      game::GameLanguage &language) {
  const auto path = settingsPath(false);
  const auto width =
      readProfileInteger(path, L"Graphics", L"Width", graphics.width);
  const auto height =
      readProfileInteger(path, L"Graphics", L"Height", graphics.height);
  if (width >= minimum_resolution_width &&
      height >= minimum_resolution_height) {
    graphics.width = width;
    graphics.height = height;
  }
  const auto msaa =
      readProfileInteger(path, L"Graphics", L"MSAA", graphics.msaa_samples);
  if (msaa == 0 || msaa == 2 || msaa == 4 || msaa == 8) {
    graphics.msaa_samples = msaa;
  }
  graphics.bilinear_filtering =
      readProfileInteger(path, L"Graphics", L"Bilinear",
                         graphics.bilinear_filtering ? 1 : 0) != 0;
  graphics.trilinear_filtering =
      readProfileInteger(path, L"Graphics", L"Trilinear",
                         graphics.trilinear_filtering ? 1 : 0) != 0;
  graphics.anisotropic_filtering =
      readProfileInteger(path, L"Graphics", L"Anisotropic",
                         graphics.anisotropic_filtering ? 1 : 0) != 0;
  graphics.smaa = readProfileInteger(path, L"Graphics", L"SMAA",
                                     graphics.smaa ? 1 : 0) != 0;
  if (graphics.smaa) {
    graphics.msaa_samples = 0;
  }
  graphics.volumetric_effects =
      readProfileInteger(path, L"Graphics", L"VolumetricEffects",
                         graphics.volumetric_effects ? 1 : 0) == 1;
  graphics.mission_skyboxes =
      readProfileInteger(path, L"Graphics", L"MissionSkyboxes",
                         graphics.mission_skyboxes ? 1 : 0) == 1;
  graphics.vsync = readProfileInteger(path, L"Graphics", L"VSync",
                                      graphics.vsync ? 1 : 0) != 0;
  const auto frame_limit = readProfileInteger(
      path, L"Graphics", L"FrameLimit", static_cast<int>(graphics.frame_limit));
  if (frame_limit == 0 || (frame_limit >= 20 && frame_limit <= 1000)) {
    graphics.frame_limit = static_cast<std::uint32_t>(frame_limit);
  }
  graphics.fullscreen = readProfileInteger(path, L"Graphics", L"Fullscreen",
                                           graphics.fullscreen ? 1 : 0) != 0;
  graphics.aspect_ratio =
      readProfileInteger(
          path, L"Graphics", L"Aspect",
          graphics.aspect_ratio == AspectRatioMode::adaptive ? 0 : 1) == 0
          ? AspectRatioMode::adaptive
          : AspectRatioMode::original_4_3;
  const auto controller_protocol =
      readProfileInteger(path, L"Controller", L"Protocol", 0);
  if (controller_protocol >= static_cast<int>(ControllerProtocol::automatic) &&
      controller_protocol <= static_cast<int>(ControllerProtocol::raw_input)) {
    graphics.controller_protocol =
        static_cast<ControllerProtocol>(controller_protocol);
  }
  graphics.controller_vibration =
      readProfileInteger(path, L"Controller", L"Vibration",
                         graphics.controller_vibration ? 1 : 0) != 0;
  language = readProfileInteger(path, L"Game", L"Locale", 0) == 1
                 ? game::GameLanguage::russian_vit
                 : game::GameLanguage::english;

  for (std::size_t index = 0U; index < keyboard_mouse_action_count; ++index) {
    const auto action = static_cast<KeyboardMouseAction>(index);
    const auto key = widenAscii(keyboardMouseActionConfigKey(action));
    const auto fallback = static_cast<int>(input[action]);
    const auto loaded = static_cast<KeyboardMouseInput>(
        readProfileInteger(path, L"KeyboardMouse", key.c_str(), fallback));
    if (isValidKeyboardMouseInput(loaded)) {
      input[action] = loaded;
    }
  }

  auto loaded_controller = graphics.controller_bindings;
  for (const auto &metadata : game::controllerActionCatalog()) {
    const auto key = widenAscii(metadata.config_key);
    const auto loaded = static_cast<std::uint32_t>(readProfileInteger(
        path, L"ControllerBindings", key.c_str(),
        static_cast<int>(loaded_controller[metadata.action])));
    loaded_controller[metadata.action] = loaded;
  }
  loaded_controller.stick_layout = static_cast<game::ControllerStickLayout>(
      readProfileInteger(path, L"ControllerBindings", L"StickLayout",
                         static_cast<int>(loaded_controller.stick_layout)));
  if (game::areControllerBindingsValid(loaded_controller)) {
    graphics.controller_bindings = loaded_controller;
  }
}

bool saveSettingsFile(const GraphicsSettings &graphics,
                      const KeyboardMouseBindings &input,
                      game::GameLanguage language,
                      const std::filesystem::path &cue_path) {
  if (!game::areControllerBindingsValid(graphics.controller_bindings)) {
    return false;
  }
  bool directory_ready{};
  const auto path = settingsPath(true, &directory_ready);
  if (!directory_ready) {
    return false;
  }

  static_cast<void>(
      writeProfileInteger(path, L"Graphics", L"Width", graphics.width));
  static_cast<void>(
      writeProfileInteger(path, L"Graphics", L"Height", graphics.height));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"MSAA",
      graphics.smaa ? 0 : graphics.msaa_samples));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"Bilinear",
      graphics.bilinear_filtering ? 1 : 0));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"Trilinear",
      graphics.trilinear_filtering ? 1 : 0));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"Anisotropic",
      graphics.anisotropic_filtering ? 1 : 0));
  static_cast<void>(
      writeProfileInteger(path, L"Graphics", L"SMAA", graphics.smaa ? 1 : 0));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"VolumetricEffects",
      graphics.volumetric_effects ? 1 : 0));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"MissionSkyboxes",
      graphics.mission_skyboxes ? 1 : 0));
  static_cast<void>(
      writeProfileInteger(path, L"Graphics", L"VSync", graphics.vsync ? 1 : 0));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"FrameLimit",
      static_cast<int>(graphics.frame_limit)));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"Fullscreen", graphics.fullscreen ? 1 : 0));
  static_cast<void>(writeProfileInteger(
      path, L"Graphics", L"Aspect",
      graphics.aspect_ratio == AspectRatioMode::adaptive ? 0 : 1));
  static_cast<void>(
      writeProfileInteger(path, L"Controller", L"Protocol",
                          static_cast<int>(graphics.controller_protocol)));
  static_cast<void>(writeProfileInteger(
      path, L"Controller", L"Vibration",
      graphics.controller_vibration ? 1 : 0));
  static_cast<void>(
      writeProfileInteger(path, L"Game", L"Locale",
                          language == game::GameLanguage::russian_vit ? 1 : 0));
  for (std::size_t index = 0U; index < keyboard_mouse_action_count; ++index) {
    const auto action = static_cast<KeyboardMouseAction>(index);
    const auto key = widenAscii(keyboardMouseActionConfigKey(action));
    static_cast<void>(writeProfileInteger(
        path, L"KeyboardMouse", key.c_str(),
        static_cast<int>(input[action])));
  }
  static_cast<void>(
      writeControllerBindingsFile(path, graphics.controller_bindings));
  static_cast<void>(saveGameImagePath(path, cue_path));
  // With all string arguments null this call is a cache-flush command. Its
  // return value is not a write result, so verify the persisted snapshot.
  static_cast<void>(
      WritePrivateProfileStringW(nullptr, nullptr, nullptr, path.c_str()));
  return settingsSnapshotMatches(path, graphics, input, language, cue_path);
}

bool saveControllerSettingsFile(const ControllerButtonBindings &bindings,
                                bool vibration) {
  if (!game::areControllerBindingsValid(bindings)) {
    return false;
  }
  bool directory_ready{};
  const auto path = settingsPath(true, &directory_ready);
  if (!directory_ready) {
    return false;
  }
  static_cast<void>(
      writeProfileInteger(path, L"Controller", L"Vibration", vibration ? 1 : 0));
  static_cast<void>(writeControllerBindingsFile(path, bindings));
  static_cast<void>(
      WritePrivateProfileStringW(nullptr, nullptr, nullptr, path.c_str()));
  bool matches = profileIntegerEquals(path, L"Controller", L"Vibration",
                                      vibration ? 1 : 0);
  for (const auto &metadata : game::controllerActionCatalog()) {
    const auto key = widenAscii(metadata.config_key);
    matches = profileIntegerEquals(
                  path, L"ControllerBindings", key.c_str(),
                  static_cast<int>(bindings[metadata.action])) &&
              matches;
  }
  matches = profileIntegerEquals(
                path, L"ControllerBindings", L"StickLayout",
                static_cast<int>(bindings.stick_layout)) &&
            matches;
  return matches;
}

bool cheatMarkerExists() noexcept {
  try {
    std::error_code error;
    return std::filesystem::exists(
               executableDirectory() / L"syphon_filter_cheats", error) &&
           !error;
  } catch (...) {
    return false;
  }
}

} // namespace sf::platform::launcher_settings

#else

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unistd.h>

namespace sf::platform::launcher_settings {
namespace {

constexpr int minimum_resolution_width = 320;
constexpr int minimum_resolution_height = 240;

std::string trim(std::string_view str) {
  while (!str.empty() && std::isspace(static_cast<unsigned char>(str.front()))) {
    str.remove_prefix(1);
  }
  while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back()))) {
    str.remove_suffix(1);
  }
  return std::string{str};
}

class IniDocument {
public:
  std::map<std::string, std::map<std::string, std::string>> sections;

  static IniDocument load(const std::filesystem::path &path) {
    IniDocument doc;
    std::ifstream file(path);
    if (!file.is_open()) {
      return doc;
    }
    std::string line;
    std::string current_section;
    while (std::getline(file, line)) {
      auto trimmed = trim(line);
      if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#') {
        continue;
      }
      if (trimmed.front() == '[' && trimmed.back() == ']') {
        current_section = trim(trimmed.substr(1, trimmed.size() - 2));
      } else if (!current_section.empty()) {
        const auto eq = trimmed.find('=');
        if (eq != std::string::npos) {
          const auto key = trim(trimmed.substr(0, eq));
          const auto val = trim(trimmed.substr(eq + 1));
          doc.sections[current_section][key] = val;
        }
      }
    }
    return doc;
  }

  bool save(const std::filesystem::path &path) const {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
      return false;
    }
    for (const auto &[section, keys] : sections) {
      file << "[" << section << "]\n";
      for (const auto &[key, val] : keys) {
        file << key << "=" << val << "\n";
      }
      file << "\n";
    }
    return file.good();
  }

  int getInt(const std::string &section, const std::string &key, int fallback) const {
    const auto sec_it = sections.find(section);
    if (sec_it == sections.end()) return fallback;
    const auto key_it = sec_it->second.find(key);
    if (key_it == sec_it->second.end()) return fallback;
    try {
      return std::stoi(key_it->second);
    } catch (...) {
      return fallback;
    }
  }

  std::string getString(const std::string &section, const std::string &key, const std::string &fallback) const {
    const auto sec_it = sections.find(section);
    if (sec_it == sections.end()) return fallback;
    const auto key_it = sec_it->second.find(key);
    if (key_it == sec_it->second.end()) return fallback;
    return key_it->second;
  }

  void setInt(const std::string &section, const std::string &key, int value) {
    sections[section][key] = std::to_string(value);
  }

  void setString(const std::string &section, const std::string &key, const std::string &value) {
    sections[section][key] = value;
  }
};

std::filesystem::path settingsPath(bool create_directory, bool *directory_ready = nullptr) {
  std::filesystem::path directory;
  const char *files_dir = std::getenv("FILESDIR");
  const char *app_data = std::getenv("SF_DATA_DIR");
  const char *home = std::getenv("HOME");

  if (app_data && *app_data) {
    directory = std::filesystem::path{app_data};
  } else if (files_dir && *files_dir) {
    directory = std::filesystem::path{files_dir} / "SyphonFilterPC";
  } else if (home && *home) {
    directory = std::filesystem::path{home} / ".config" / "SyphonFilterPC";
  } else {
    directory = std::filesystem::current_path();
  }

  if (create_directory) {
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    const auto is_directory = !error && std::filesystem::is_directory(directory, error);
    if (directory_ready != nullptr) {
      *directory_ready = is_directory && !error;
    }
  } else if (directory_ready != nullptr) {
    *directory_ready = true;
  }
  return directory / "launcher.ini";
}

} // namespace

std::filesystem::path executableDirectory() {
  try {
    std::error_code ec;
    auto exe_path = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec && !exe_path.empty()) {
      return exe_path.parent_path();
    }
  } catch (...) {}
  return std::filesystem::current_path();
}

std::filesystem::path loadGameImagePath() {
  const auto path = settingsPath(false);
  auto doc = IniDocument::load(path);
  auto img = doc.getString("Game", "Image", "");
  if (img.empty()) return {};
  return std::filesystem::path{img};
}

void loadSettingsFile(GraphicsSettings &graphics, KeyboardMouseBindings &input,
                      game::GameLanguage &language) {
  const auto path = settingsPath(false);
  auto doc = IniDocument::load(path);

  const auto width = doc.getInt("Graphics", "Width", graphics.width);
  const auto height = doc.getInt("Graphics", "Height", graphics.height);
  if (width >= minimum_resolution_width && height >= minimum_resolution_height) {
    graphics.width = width;
    graphics.height = height;
  }
  const auto msaa = doc.getInt("Graphics", "MSAA", graphics.msaa_samples);
  if (msaa == 0 || msaa == 2 || msaa == 4 || msaa == 8) {
    graphics.msaa_samples = msaa;
  }
  graphics.bilinear_filtering = doc.getInt("Graphics", "Bilinear", graphics.bilinear_filtering ? 1 : 0) != 0;
  graphics.trilinear_filtering = doc.getInt("Graphics", "Trilinear", graphics.trilinear_filtering ? 1 : 0) != 0;
  graphics.anisotropic_filtering = doc.getInt("Graphics", "Anisotropic", graphics.anisotropic_filtering ? 1 : 0) != 0;
  graphics.smaa = doc.getInt("Graphics", "SMAA", graphics.smaa ? 1 : 0) != 0;
  if (graphics.smaa) {
    graphics.msaa_samples = 0;
  }
  graphics.volumetric_effects = doc.getInt("Graphics", "VolumetricEffects", graphics.volumetric_effects ? 1 : 0) == 1;
  graphics.mission_skyboxes = doc.getInt("Graphics", "MissionSkyboxes", graphics.mission_skyboxes ? 1 : 0) == 1;
  graphics.vsync = doc.getInt("Graphics", "VSync", graphics.vsync ? 1 : 0) != 0;
  const auto frame_limit = doc.getInt("Graphics", "FrameLimit", static_cast<int>(graphics.frame_limit));
  if (frame_limit == 0 || (frame_limit >= 20 && frame_limit <= 1000)) {
    graphics.frame_limit = static_cast<std::uint32_t>(frame_limit);
  }
  graphics.fullscreen = doc.getInt("Graphics", "Fullscreen", graphics.fullscreen ? 1 : 0) != 0;
  graphics.aspect_ratio = doc.getInt("Graphics", "Aspect", graphics.aspect_ratio == AspectRatioMode::adaptive ? 0 : 1) == 0
                              ? AspectRatioMode::adaptive
                              : AspectRatioMode::original_4_3;

  const auto controller_protocol = doc.getInt("Controller", "Protocol", 0);
  if (controller_protocol >= static_cast<int>(ControllerProtocol::automatic) &&
      controller_protocol <= static_cast<int>(ControllerProtocol::raw_input)) {
    graphics.controller_protocol = static_cast<ControllerProtocol>(controller_protocol);
  }
  graphics.controller_vibration = doc.getInt("Controller", "Vibration", graphics.controller_vibration ? 1 : 0) != 0;
  language = doc.getInt("Game", "Locale", 0) == 1
                 ? game::GameLanguage::russian_vit
                 : game::GameLanguage::english;

  for (std::size_t index = 0U; index < keyboard_mouse_action_count; ++index) {
    const auto action = static_cast<KeyboardMouseAction>(index);
    const auto key = keyboardMouseActionConfigKey(action);
    const auto fallback = static_cast<int>(input[action]);
    const auto loaded = static_cast<KeyboardMouseInput>(doc.getInt("KeyboardMouse", key, fallback));
    if (isValidKeyboardMouseInput(loaded)) {
      input[action] = loaded;
    }
  }

  auto loaded_controller = graphics.controller_bindings;
  for (const auto &metadata : game::controllerActionCatalog()) {
    const auto loaded = static_cast<std::uint32_t>(doc.getInt("ControllerBindings", metadata.config_key, static_cast<int>(loaded_controller[metadata.action])));
    loaded_controller[metadata.action] = loaded;
  }
  loaded_controller.stick_layout = static_cast<game::ControllerStickLayout>(
      doc.getInt("ControllerBindings", "StickLayout", static_cast<int>(loaded_controller.stick_layout)));
  if (game::areControllerBindingsValid(loaded_controller)) {
    graphics.controller_bindings = loaded_controller;
  }
}

bool saveSettingsFile(const GraphicsSettings &graphics,
                      const KeyboardMouseBindings &input,
                      game::GameLanguage language,
                      const std::filesystem::path &cue_path) {
  if (!game::areControllerBindingsValid(graphics.controller_bindings)) {
    return false;
  }
  bool directory_ready{};
  const auto path = settingsPath(true, &directory_ready);
  if (!directory_ready) {
    return false;
  }
  auto doc = IniDocument::load(path);
  doc.setInt("Graphics", "Width", graphics.width);
  doc.setInt("Graphics", "Height", graphics.height);
  doc.setInt("Graphics", "MSAA", graphics.smaa ? 0 : graphics.msaa_samples);
  doc.setInt("Graphics", "Bilinear", graphics.bilinear_filtering ? 1 : 0);
  doc.setInt("Graphics", "Trilinear", graphics.trilinear_filtering ? 1 : 0);
  doc.setInt("Graphics", "Anisotropic", graphics.anisotropic_filtering ? 1 : 0);
  doc.setInt("Graphics", "SMAA", graphics.smaa ? 1 : 0);
  doc.setInt("Graphics", "VolumetricEffects", graphics.volumetric_effects ? 1 : 0);
  doc.setInt("Graphics", "MissionSkyboxes", graphics.mission_skyboxes ? 1 : 0);
  doc.setInt("Graphics", "VSync", graphics.vsync ? 1 : 0);
  doc.setInt("Graphics", "FrameLimit", static_cast<int>(graphics.frame_limit));
  doc.setInt("Graphics", "Fullscreen", graphics.fullscreen ? 1 : 0);
  doc.setInt("Graphics", "Aspect", graphics.aspect_ratio == AspectRatioMode::adaptive ? 0 : 1);

  doc.setInt("Controller", "Protocol", static_cast<int>(graphics.controller_protocol));
  doc.setInt("Controller", "Vibration", graphics.controller_vibration ? 1 : 0);
  doc.setInt("Game", "Locale", language == game::GameLanguage::russian_vit ? 1 : 0);
  doc.setString("Game", "Image", cue_path.string());

  for (std::size_t index = 0U; index < keyboard_mouse_action_count; ++index) {
    const auto action = static_cast<KeyboardMouseAction>(index);
    const auto key = keyboardMouseActionConfigKey(action);
    doc.setInt("KeyboardMouse", key, static_cast<int>(input[action]));
  }

  for (const auto &metadata : game::controllerActionCatalog()) {
    doc.setInt("ControllerBindings", metadata.config_key, static_cast<int>(graphics.controller_bindings[metadata.action]));
  }
  doc.setInt("ControllerBindings", "StickLayout", static_cast<int>(graphics.controller_bindings.stick_layout));

  return doc.save(path);
}

bool saveControllerSettingsFile(const ControllerButtonBindings &bindings,
                                bool vibration) {
  if (!game::areControllerBindingsValid(bindings)) {
    return false;
  }
  bool directory_ready{};
  const auto path = settingsPath(true, &directory_ready);
  if (!directory_ready) {
    return false;
  }
  auto doc = IniDocument::load(path);
  doc.setInt("Controller", "Vibration", vibration ? 1 : 0);
  for (const auto &metadata : game::controllerActionCatalog()) {
    doc.setInt("ControllerBindings", metadata.config_key, static_cast<int>(bindings[metadata.action]));
  }
  doc.setInt("ControllerBindings", "StickLayout", static_cast<int>(bindings.stick_layout));
  return doc.save(path);
}

bool cheatMarkerExists() noexcept {
  try {
    std::error_code error;
    if (std::filesystem::exists(executableDirectory() / "syphon_filter_cheats", error) && !error) {
      return true;
    }
    const auto pref = settingsPath(false).parent_path();
    if (std::filesystem::exists(pref / "syphon_filter_cheats", error) && !error) {
      return true;
    }
  } catch (...) {}
  return false;
}

} // namespace sf::platform::launcher_settings

#endif
