/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include <array>
#include <memory>

#include "IconsFontAwesome4.h"
#include "simdjson.h"

namespace Project { class Object; }

namespace Project::Component
{
  struct Entry
  {
    int id{};
    uint64_t uuid{};
    std::string name{};
    std::shared_ptr<void> data{};
  };

  typedef void(*FuncCompDraw)(Object&, Entry &entry);
  typedef std::shared_ptr<void>(*FuncCompInit)(Object&);
  typedef std::string(*FuncCompSerial)(Entry &entry);
  typedef std::shared_ptr<void>(*FuncCompDeserial)(simdjson::simdjson_result<simdjson::dom::object> &doc);

  struct CompInfo
  {
    int id{};
    const char* icon{};
    const char* name{};
    FuncCompInit funcInit{};
    FuncCompDraw funcDraw{};
    FuncCompSerial funcSerialize{};
    FuncCompDeserial funcDeserialize{};
  };

  #define MAKE_COMP(name) \
    namespace name \
    { \
      std::shared_ptr<void> init(Object& obj); \
      void draw(Object& obj, Entry &entry); \
      std::string serialize(Entry &entry); \
      std::shared_ptr<void> deserialize(simdjson::simdjson_result<simdjson::dom::object> &doc); \
    }

  MAKE_COMP(Code)

  constexpr std::array TABLE{
    CompInfo{
      .id = 0,
      .icon = ICON_FA_CODE " ",
      .name = "Code",
      .funcInit = Code::init,
      .funcDraw = Code::draw,
      .funcSerialize = Code::serialize,
      .funcDeserialize = Code::deserialize
    },
  };
}
