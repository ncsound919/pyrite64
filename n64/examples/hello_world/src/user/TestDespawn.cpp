#include "script/userScript.h"
#include "scene/sceneManager.h"

namespace P64::Script::C54E2E8B498612FE
{
  P64_DATA(
    // Put your arguments here if needed, those will show up in the editor.
    //
    // Allowed types:
    // - uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t
    // - float
    [[P64::Name("Group Off")]]
    uint16_t groupOff = 0;

    [[P64::Name("Group On")]]
    uint16_t groupOn = 0;
  );

  void update(Object& obj, Data *data)
  {
    auto pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if(pressed.z)
    {
      const auto &sc = SceneManager::getCurrent();
      sc.setGroupEnabled(data->groupOff, false);
      sc.setGroupEnabled(data->groupOn, true);
      obj.remove();
    }
  }
}
