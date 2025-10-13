#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inNorm;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 v_color;

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
  gl_Position = matMVP * vec4(inPosition, 1.0);

  v_color = inColor;// * vec4(test, 1.0f);
}
