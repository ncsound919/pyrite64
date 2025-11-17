#include "script/userScript.h"
#include "scene/sceneManager.h"

namespace P64::Script::C83B13C5A5C41AF3
{
  P64_DATA(
    // Put your arguments here if needed, those will show up in the editor.
    //
    // Allowed types:
    // - uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t
    // - float
  );

  void update(Object& obj, Data *data)
  {
  }

  void draw(Object& obj, Data *data)
  {
    rdpq_set_env_color({0x80, 0x80, 0x80, 0x80});
  }
}
