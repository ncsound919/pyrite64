/**
* @copyright 2026 - Max Bebök
* @license MIT
*/
#pragma once

#include "baseNode.h"
#include "../../../utils/hash.h"

namespace Project::Graph::Node
{
  class ObjDel : public Base
  {
    private:
      uint16_t objectId{};

    public:
      ObjDel()
      {
        uuid = Utils::Hash::randomU64();
        outputCount = 1;
        setTitle("Delete Object");
        setStyle(std::make_shared<ImFlow::NodeStyle>(IM_COL32(90,191,93,255), ImColor(0,0,0,255), 3.5f));

        addIN<int>("", 0, ImFlow::ConnectionFilter::SameType());
        addOUT<int>("", nullptr)->behaviour([this]() { return 42; });
      }

      void draw() override {
        ImGui::SetNextItemWidth(70.f);
        ImGui::InputScalar("##ObjectID", ImGuiDataType_U16, &objectId);
      }

      void serialize(nlohmann::json &j) override {
        j["objectId"] = objectId;
      }

      void deserialize(nlohmann::json &j) override {
        objectId = j["objectId"];
      }

      void build(Utils::BinaryFile &f) override {
        f.write<uint16_t>(objectId);
      }
  };
}