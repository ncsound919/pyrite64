/**
* @copyright 2026 - Max Bebök
* @license MIT
*/
#pragma once
#include "../../../utils/prop.h"

namespace Project::Component::Shared
{
  struct Material
  {
    PROP_BOOL(setDepthRead);
    PROP_BOOL(depthRead);

    PROP_BOOL(setDepthWrite);
    PROP_BOOL(depthWrite);
  };
}