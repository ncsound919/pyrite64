/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include "parts/assetInspector.h"
#include "parts/assetsBrowser.h"
#include "parts/logWindow.h"
#include "parts/objectInspector.h"
#include "parts/projectSettings.h"
#include "parts/sceneBrowser.h"
#include "parts/sceneGraph.h"
#include "parts/sceneInspector.h"
#include "parts/viewport3D.h"

namespace Editor
{
  class Scene
  {
    private:
      Viewport3D viewport3d{};

      // Editors
      ProjectSettings projectSettings{};
      AssetsBrowser assetsBrowser{};
      SceneBrowser sceneBrowser{};
      AssetInspector assetInspector{};
      SceneInspector sceneInspector{};
      ObjectInspector objectInspector{};
      LogWindow logWindow{};
      SceneGraph sceneGraph{};

      bool dockSpaceInit{false};

    public:
      void draw();
  };
}
