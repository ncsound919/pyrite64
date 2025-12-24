/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include "fs.h"
#include "simdjson.h"
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Utils::JSON
{
  inline simdjson::dom::parser parser{};

  inline simdjson::simdjson_result<simdjson::dom::element> load(const std::string &json) {
    auto doc = parser.parse(std::string_view{json});
    return doc;
  }

  inline simdjson::simdjson_result<simdjson::dom::element> loadFile(const std::string &path) {
    auto jsonData = FS::loadTextFile(path);

    if (jsonData.empty()) {
      return {};
    }

    auto doc = parser.parse(std::string_view{jsonData});
    return doc;
  }

  template<typename T>
  std::string readString(const simdjson::simdjson_result<T> &el, const std::string &key, const std::string &defaultValue = "") {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS) {
      return defaultValue;
    }
    auto str = val.get_string();
    if (str.error() != simdjson::SUCCESS) {
      return defaultValue;
    }
    return std::string{str->data(), str->length()};
  }

  template<typename T>
  inline int readInt(const simdjson::simdjson_result<T> &el, const std::string &key, int def = 0) {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS) {
      return def;
    }
    auto i = val.get_int64();
    if (i.error() != simdjson::SUCCESS) {
      return def;
    }
    return (int)(*i);
  }

  template<typename T>
  inline uint64_t readU64(const simdjson::simdjson_result<T> &el, const std::string &key, uint64_t def = 0) {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS) {
      return def;
    }
    auto i = val.get_uint64();
    if (i.error() != simdjson::SUCCESS) {
      return def;
    }
    return *i;
  }

  template<typename T>
  inline uint32_t readU32(const simdjson::simdjson_result<T> &el, const std::string &key, uint32_t def = 0)
  {
    return (uint32_t)readU64(el, key, def);
  }

  template<typename T>
  inline float readFloat(const simdjson::simdjson_result<T> &el, const std::string &key, float def = 0.0f) {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS) {
      return def;
    }
    auto f = val.get_double();
    if (f.error() != simdjson::SUCCESS) {
      return def;
    }
    return (float)(*f);
  }

  template<typename T>
  inline bool readBool(const simdjson::simdjson_result<T> &el, const std::string &key, bool defValue = false) {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS) {
      return defValue;
    }
    auto b = val.get_bool();
    if (b.error() != simdjson::SUCCESS) {
      return defValue;
    }
    return *b;
  }

  template<typename T>
  inline glm::vec4 readColor(const simdjson::simdjson_result<T> &el, const std::string &key) {
    glm::vec4 col{};
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS) {
      return col;
    }
    auto arr = val.get_array();
    if (arr.error() != simdjson::SUCCESS) {
      return col;
    }

    col.r = arr.at(0).get_double();
    col.g = arr.at(1).get_double();
    col.b = arr.at(2).get_double();
    col.a = arr.at(3).get_double();
    return col;
  }

  template<typename T>
  inline glm::vec2 readVec2(const simdjson::simdjson_result<T> &el, const std::string &key, const glm::vec3 &def = {}) {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS)return def;
    auto arr = val.get_array();
    if (arr.error() != simdjson::SUCCESS)return def;

    return {
      (float)arr.at(0).get_double(),
      (float)arr.at(1).get_double(),
    };
  }

  template<typename T>
  inline glm::vec3 readVec3(const simdjson::simdjson_result<T> &el, const std::string &key, const glm::vec3 &def = {}) {
    auto val = el[key];
    if (val.error() != simdjson::SUCCESS)return def;
    auto arr = val.get_array();
    if (arr.error() != simdjson::SUCCESS)return def;

    return {
      (float)arr.at(0).get_double(),
      (float)arr.at(1).get_double(),
      (float)arr.at(2).get_double()
    };
  }

  inline glm::quat readQuat(const simdjson::simdjson_result<simdjson::dom::element> &el, const std::string &key) {
    auto res = glm::identity<glm::quat>();

    auto val = el[key];
    if (val.error() != simdjson::SUCCESS)return res;
    auto arr = val.get_array();
    if (arr.error() != simdjson::SUCCESS)return res;

    res.x = arr.at(0).get_double();
    res.y = arr.at(1).get_double();
    res.z = arr.at(2).get_double();
    res.w = arr.at(3).get_double();
    return res;
  }

  template<typename T, typename PROP>
  inline void readProp(const simdjson::simdjson_result<T> &el, Property<PROP> &prop, const PROP& defValue = PROP{}) {
    PROP val{};
    if constexpr (std::is_same_v<PROP, bool>) {
      val = readBool(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, uint32_t>) {
      val = readU32(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, uint64_t>) {
      val = readU64(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, int32_t>) {
      val = readInt(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, int64_t>) {
      val = (int64_t)readInt(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, float>) {
      val = readFloat(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, glm::vec3>) {
      val = readVec3(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, glm::vec4>) {
      val = readColor(el, prop.name, defValue);
    } else if constexpr (std::is_same_v<PROP, glm::quat>) {
      val = readQuat(el, prop.name);
    } else if constexpr (std::is_same_v<PROP, std::string>) {
      val = readString(el, prop.name, defValue);
    } else
    {
      static_assert(false, "Unsupported type in Utils::JSON::readProp");
    }

    prop.value = val;
  }
}
