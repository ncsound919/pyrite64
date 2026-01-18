/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

#include "lib/types.h"

namespace P64
{
  struct Camera
  {
    T3DViewport viewports{};
    fm_vec3_t up{0,1,0};
    fm_vec3_t pos{};
    fm_vec3_t target{};
    float fov{};
    float near{};
    float far{};
    float aspectRatio{};

    uint8_t needsProjUpdate{false};

    Camera() = default;
    CLASS_NO_COPY_MOVE(Camera);

    void update(float deltaTime);
    void attach();

    void setUp(fm_vec3_t newUp) {
      up = newUp;
    }

    void setPos(fm_vec3_t newPos) {
      pos = newPos;
    }

    void setTarget(fm_vec3_t newPos) {
      target = newPos;
    }

    void move(fm_vec3_t dir) {
      target += dir;
      pos += dir;
    }

    [[nodiscard]] const fm_vec3_t &getTarget() const { return target; }
    [[nodiscard]] const fm_vec3_t &getPos() const { return pos; }

    [[nodiscard]] fm_vec3_t getViewDir() const {
      fm_vec3_t dir{};
      fm_vec3_sub(&dir, &target, &pos);
      fm_vec3_norm(&dir, &dir);
      return dir;
    }

    fm_vec3_t getScreenPos(const fm_vec3_t &worldPos);
  };
}
