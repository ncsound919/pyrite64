#version 460

layout (location = 0) in ivec3 inPosition;
//layout (location = 1) in vec2 inNorm;
layout (location = 1) in vec4 inColor;
layout (location = 2) in ivec2 inUV;

layout (location = 0) out vec4 v_color;
layout (location = 1) out vec2 v_uv;

// set=3 in fragment shader
layout(std140, set = 1, binding = 0) uniform UniformGlobal {
    mat4 projMat;
    mat4 cameraMat;
};

layout(std140, set = 1, binding = 1) uniform UniformObject {
    mat4 modelMat;
};

void main()
{
  mat4 matMVP = projMat * cameraMat * modelMat;
  vec3 posNorm = vec3(inPosition) / 32768.0;
  //posNorm /= 32768.0;
  posNorm *= 64;

  gl_Position = matMVP * vec4(posNorm, 1.0);

  v_color = inColor;// * vec4(test, 1.0f);
  v_uv = vec2(inUV) / float(1 << 5);
}
