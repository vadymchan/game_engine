#ifndef GAME_ENGINE_CONFIG_H
#define GAME_ENGINE_CONFIG_H

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <any>
#include <filesystem>
#include <future>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace game_engine {

using ConfigValue = rapidjson::Value;

// TODO: remove this concept
template <typename T>
concept IsArray = std::is_same_v<T, std::vector<typename T::value_type>>;

// TODO: remove this concept
template <typename T>
concept IsNotArray = !IsArray<T>;

template <typename T>
concept HasEmplaceBack = requires(T container, typename T::value_type value) {
  container.emplace_back(value);
};

template <typename T>
concept HasEmplaceFront = requires(T container, typename T::value_type value) {
  container.emplace_front(value);
};

template <typename T>
concept HasEmplace = requires(T container, typename T::value_type value) {
  container.emplace(value);
};

template <typename T>
concept SupportsEmplace
    = HasEmplaceBack<T> || HasEmplaceFront<T> || HasEmplace<T>;

class Config {
  public:
  Config()  = default;
  ~Config() = default;

  void reloadAsync();

  void loadFromFileAsync(const std::filesystem::path& filePath);

  bool loadFromFile(const std::filesystem::path& filePath);

  std::string toString() const;

  [[nodiscard]] const std::filesystem::path& getFilename() const;

  template <typename T>
    requires IsNotArray<T>
  T get(const std::string& key) const {
    asyncLoadComplete_();
    if (!m_root_.IsObject()) {
      std::cerr << "Error: Configuration not loaded or root is not an object."
                << std::endl;
      return T();
    }

    const ConfigValue& value = getMember_(key);
    return convert_<T>(value);
  }

  template <SupportsEmplace Container,
            typename T = typename Container::value_type>
  Container getContainer(const std::string& key) const {
    asyncLoadComplete_();
    if (!m_root_.IsObject()) {
      std::cerr << "Error: Configuration not loaded or root is not an object."
                << std::endl;
      return Container();
    }

    const ConfigValue& value = getMember_(key);
    if (!value.IsArray()) {
      std::cerr << "Error: Value for key \"" << key << "\" is not an array."
                << std::endl;
      return Container();
    }

    Container container;
    for (const auto& v : value.GetArray()) {
      /// @brief Check if the container supports `emplace_back` (vector, deque,
      /// list).
      if constexpr (HasEmplaceBack<Container>) {
        container.emplace_back(convert_<T>(v));
      }
      /// @brief Check if the container supports `emplace_front` (forward_list).
      else if constexpr (HasEmplaceFront<Container>) {
        container.emplace_front(convert_<T>(v));
      }
      /// @brief Check if the container supports `emplace` (set, unordered_set,
      /// stack, queue, priority_queue).
      else if constexpr (HasEmplace<Container>) {
        container.emplace(convert_<T>(v));
      }
    }

    return container;
  }

  template <typename T>
  void registerConverter(std::function<T(const ConfigValue&)> converter) {
    auto test = std::type_index(typeid(T));
    m_converters_[std::type_index(typeid(T))]
        = [converter](const ConfigValue& value) -> std::any {
      return converter(value);
    };
  }

  private:
  void asyncLoadComplete_() const;

  const ConfigValue& getMember_(const std::string& key) const;

  template <typename T>
  T convert_(const ConfigValue& value) const {
    if (value.IsObject()) {
      return convertObject_<T>(value);
    }
    std::cerr << "Error: Value is not an object." << std::endl;
    return T();
  }

  template <typename T>
  T convertObject_(const ConfigValue& value) const {
    auto it = m_converters_.find(std::type_index(typeid(T)));
    if (it != m_converters_.end()) {
      std::any result = it->second(value);
      if (result.has_value()) {
        return std::any_cast<T>(result);
      }
    } else {
      std::cerr << "Error: No converter registered for the requested type."
                << std::endl;
    }
    return T();
  }

  template <>
  bool convert_<bool>(const ConfigValue& value) const {
    if (value.IsBool()) {
      return value.GetBool();
    }
    std::cerr << "Type error: Value is not a bool." << std::endl;
    return false;
  }

  template <>
  float convert_<float>(const ConfigValue& value) const {
    // treat all numeric values as float
    if (value.IsNumber()) {
      return value.GetFloat();
    }
    std::cerr << "Type error: Value is not a float." << std::endl;
    return 0.0f;
  }

  template <>
  double convert_<double>(const ConfigValue& value) const {
    // treat all numeric values as double
    if (value.IsNumber()) {
      return value.GetDouble();
    }
    std::cerr << "Type error: Value is not a double." << std::endl;
    return 0.0;
  }

  template <>
  std::uint32_t convert_<std::uint32_t>(const ConfigValue& value) const {
    if (value.IsUint()) {
      return value.GetUint();
    }
    std::cerr << "Type error: Value is not a uint32." << std::endl;
    return 0;
  }

  template <>
  std::int32_t convert_<std::int32_t>(const ConfigValue& value) const {
    if (value.IsInt()) {
      return value.GetInt();
    }
    std::cerr << "Type error: Value is not an int32." << std::endl;
    return 0;
  }

  template <>
  std::uint64_t convert_<std::uint64_t>(const ConfigValue& value) const {
    if (value.IsUint64()) {
      return value.GetUint64();
    }
    std::cerr << "Type error: Value is not a uint64." << std::endl;
    return 0;
  }

  template <>
  std::int64_t convert_<std::int64_t>(const ConfigValue& value) const {
    if (value.IsInt64()) {
      return value.GetInt64();
    }
    std::cerr << "Type error: Value is not an int64." << std::endl;
    return 0;
  }

  template <>
  std::string convert_<std::string>(const ConfigValue& value) const {
    if (value.IsString()) {
      return value.GetString();
    }
    std::cerr << "Type error: Value is not a string." << std::endl;
    return "";
  }

  std::filesystem::path     m_filePath_;
  rapidjson::Document       m_root_;
  mutable std::future<bool> m_future_;
  mutable std::unordered_map<std::type_index,
                             std::function<std::any(const ConfigValue&)>>
      m_converters_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_CONFIG_H
