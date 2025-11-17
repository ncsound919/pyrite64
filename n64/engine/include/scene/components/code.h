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
    // @TODO: only store used functions
    Script::FuncObject funcInit{};
    Script::FuncObject funcUpdate{};
    Script::FuncObject funcDraw{};
    Script::FuncObject funcDestroy{};

    static uint32_t getAllocSize(uint16_t* initData)
    {
      auto dataSize = Script::getCodeSizeByIndex(initData[0]);
      return sizeof(Code) + dataSize;
    }

    static void initDelete([[maybe_unused]] Object& obj, Code* data, uint16_t* initData)
    {
      if (initData == nullptr)return;

      auto scriptPtr = Script::getCodeByIndex(initData[0]);
      auto dataSize = Script::getCodeSizeByIndex(initData[0]);
      // reserved: initData[1];

      data->funcInit = scriptPtr.init;
      data->funcUpdate = scriptPtr.update;
      data->funcDraw = scriptPtr.draw;
      data->funcDestroy = scriptPtr.destroy;

      if (dataSize > 0) {
        memcpy((char*)data + sizeof(Code), (char*)&initData[2], dataSize);
      }

      if(data->funcInit) {
        char* funcData = (char*)data + sizeof(Code);
        data->funcInit(obj, funcData);
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