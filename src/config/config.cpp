#include "config/config.h"

#include "file_loader/file_system_manager.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
void Config::reloadAsync() {
  GlobalLogger::Log(LogLevel::Info,
                    "Reloading config due to file modification...");
  loadFromFileAsync(m_filePath_);
  return;
}

void Config::loadFromFileAsync(const std::filesystem::path& filePath) {
  // Check that previous async load is complete
  asyncLoadComplete_();

  m_future_
      = std::async(std::launch::async, &Config::loadFromFile, this, filePath);
}

bool Config::loadFromFile(const std::filesystem::path& filePath) {
  m_filePath_ = filePath;

  auto fileContent = FileSystemManager::readFile(filePath);
  if (!fileContent) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to read file: " + filePath.string());
    return false;
  }

  m_root_.Parse(fileContent->c_str());

  if (m_root_.HasParseError()) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to parse JSON in file: " + filePath.string());
    return false;
  }

  return true;
}

std::string Config::toString() const {
  asyncLoadComplete_();
  if (!m_root_.IsObject()) {
    GlobalLogger::Log(LogLevel::Error,
                      "Configuration not loaded or root is not an object.");
    return "";
  }

  rapidjson::StringBuffer                    buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  m_root_.Accept(writer);

  return buffer.GetString();
}

[[nodiscard]] const std::filesystem::path& Config::getFilename() const {
  return m_filePath_;
}

void Config::asyncLoadComplete_() const {
  if (m_future_.valid()) {
    m_future_.wait();
  }
}

const ConfigValue& Config::getMember_(const std::string& key) const {
  if (!m_root_.HasMember(key.c_str())) {
    static ConfigValue nullValue(rapidjson::kNullType);
    GlobalLogger::Log(LogLevel::Error,
                      "Key \"" + key + "\" not found in config.");
    return nullValue;
  }
  return m_root_[key.c_str()];
}
}  // namespace game_engine
