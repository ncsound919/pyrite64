set -e
cd "$(dirname "$0")"

glslc -fshader-stage=fragment main3d.frag.glsl -o main3d.frag.spv
glslc -fshader-stage=vertex main3d.vert.glsl -o main3d.vert.spv