set -e
cd "$(dirname "$0")"

glslc -fshader-stage=fragment n64.frag.glsl -o n64.frag.spv
glslc -fshader-stage=vertex n64.vert.glsl -o n64.vert.spv