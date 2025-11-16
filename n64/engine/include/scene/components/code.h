/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include "scene/object.h"
#include "script/scriptTable.h"

namespace P64::Comp
{
  struct Code
  {
    Script::FuncUpdate funcUpdate{};
    Script::FuncUpdate funcDraw{};

    static uint32_t getAllocSize(uint16_t* initData)
    {
      auto scriptPtr = Script::getCodeByIndex(initData[0]);
      assert(scriptPtr.update != nullptr);
      return sizeof(Code) + scriptPtr.dataSize;
    }

    static void initDelete([[maybe_unused]] Object& obj, Code* data, uint16_t* initData)
    {
      if (initData == nullptr)return;

      auto scriptPtr = Script::getCodeByIndex(initData[0]);
      // reserved: initData[1];

      data->funcUpdate = scriptPtr.update;
      data->funcDraw = scriptPtr.draw;

      if (scriptPtr.dataSize > 0) {
        memcpy((char*)data + sizeof(Code), (char*)&initData[2], scriptPtr.dataSize);
      }
    }

    static void update(Object& obj, Code* data) {
      char* funcData = (char*)data + sizeof(Code);
      if(data->funcUpdate)data->funcUpdate(obj, funcData);
    }

    static void draw(Object& obj, Code* data) {
      char* funcData = (char*)data + sizeof(Code);
      if(data->funcDraw)data->funcDraw(obj, funcData);
    }
  };
}