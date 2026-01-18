/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "scene/object.h"
#include "scene/components/camera.h"

#include "scene/scene.h"

void P64::Comp::Camera::initDelete(Object &obj, Camera* data, InitData* initData)
{
  if (initData == nullptr) {
    SceneManager::getCurrent().removeCamera(&data->camera);
    t3d_viewport_destroy(&data->camera.viewports);
    data->~Camera();
    return;
  }

  new(data) Camera();

  SceneManager::getCurrent().addCamera(&data->camera);
  auto &cam = data->camera;
  cam.setPos(obj.pos);
  cam.setTarget({0,0,0});
  cam.fov  = initData->fov;
  cam.near = initData->near;
  cam.far  = initData->far;

  cam.aspectRatio = initData->aspectRatio;
  if(cam.aspectRatio <= 0) {
    cam.aspectRatio = (float)initData->vpSize[0] / (float)initData->vpSize[1];
  }

  cam.viewports = t3d_viewport_create_buffered(3);
  t3d_viewport_set_area(cam.viewports,
    initData->vpOffset[0], initData->vpOffset[1],
    initData->vpSize[0], initData->vpSize[1]
  );
}
