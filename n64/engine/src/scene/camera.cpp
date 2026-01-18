/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "scene/camera.h"
#include "lib/logger.h"
#include "scene/globalState.h"

void P64::Camera::update([[maybe_unused]] float deltaTime)
{
  t3d_viewport_set_perspective(&viewports, fov, aspectRatio, near, far);
  t3d_viewport_look_at(viewports, pos, target, up);
}

void P64::Camera::attach() {
  t3d_viewport_attach(viewports);
}

fm_vec3_t P64::Camera::getScreenPos(const fm_vec3_t &worldPos)
{
  fm_vec3_t res{};
  t3d_viewport_calc_viewspace_pos(viewports, res, worldPos);
  return res;
}

