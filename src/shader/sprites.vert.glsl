#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in uint inObjectId;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec4 v_color;
layout (location = 1) out vec2 v_uv;
layout (location = 2) out flat uint v_objectID;

// set=3 in fragment shader
layout(std140, set = 1, binding = 0) uniform UniformGlobal {
    mat4 projMat;
    mat4 cameraMat;
    vec2 screenSize;
    vec2 spriteSize;
};

layout(std140, set = 1, binding = 1) uniform UniformObject {
    mat4 modelMat;
    uint objectID;
};

void main()
{
  const float SPRITE_COUNT = 1.0 / 8.0;

  int corner = gl_VertexIndex % 4;

  mat4 matMVP = projMat * cameraMat * modelMat;
  vec4 posScreen = matMVP * vec4(inPosition, 1.0);

  vec2 stepPerPixel = vec2(2.0 / screenSize.x, 2.0 / screenSize.y);
  vec2 localSpriteSize = spriteSize * stepPerPixel;
  vec2 uv = vec2(0.0);

  v_color = inColor;
  v_color.a = 1.0;

  if(corner == 0) {
    posScreen.x -= localSpriteSize.x;
    posScreen.y += localSpriteSize.y;
    uv = vec2(0.0, 0.0);
  }
  else if(corner == 1) {
    posScreen.x += localSpriteSize.x;
    posScreen.y += localSpriteSize.y;
    uv = vec2(1.0, 0.0);
  }
  else if(corner == 3) {
    posScreen.x -= localSpriteSize.x;
    posScreen.y -= localSpriteSize.y;
    uv = vec2(0.0, 1.0);
  }
  else if(corner == 2) {
    posScreen.x += localSpriteSize.x;
    posScreen.y -= localSpriteSize.y;
    uv = vec2(1.0, 1.0);
  }

  float spriteIdx = inColor.a * 255.0;
  uv.x *= SPRITE_COUNT;
  uv.x += spriteIdx * SPRITE_COUNT;

  gl_Position = posScreen;

  /*if(spriteSize.x < 100) {
    gl_Position.z = 0.5;
  }*/


  v_uv = uv;
  v_objectID = inObjectId;
}
