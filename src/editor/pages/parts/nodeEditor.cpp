/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "nodeEditor.h"

#include "imgui.h"
#include "../../../context.h"
#include "../../../utils/logger.h"
#include "../../imgui/helper.h"

#include "ImNodeFlow.h"
#include "json.hpp"
#include "../../../utils/fs.h"

ImFlow::ImNodeFlow mINF;

namespace
{

  class P64Node : public ImFlow::BaseNode
  {
    public:
      uint64_t uuid{};
      uint32_t type{};

      virtual void serialize(nlohmann::json &j) = 0;
      virtual void deserialize(nlohmann::json &j) = 0;
  };

  class SimpleSum : public P64Node {

    public:
      SimpleSum() {
        uuid = Utils::Hash::randomU64();
        setTitle("Delay (sec.)");
        setStyle(std::make_shared<ImFlow::NodeStyle>(IM_COL32(90,191,93,255), ImColor(0,0,0,255), 3.5f));
        ImFlow::BaseNode::addIN<int>("In", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this]() { return getInVal<float>("In") + m_valB; });
      }

      void draw() override {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputFloat("##ValB", &m_valB);
        //printf("aa: %f | %08X\n", m_valB, ImGui::GetItemID());
        if(ImGui::Button("aaaa"))m_valB++;
      }

      void serialize(nlohmann::json &j) override
      {
        j["valB"] = m_valB;
      }

      void deserialize(nlohmann::json &j) override
      {
        m_valB = j["valB"];
      }

    private:
      float m_valB = 0;
  };

  class ResultNode : public P64Node {
    public:
      ResultNode() {
        uuid = Utils::Hash::randomU64();
        setTitle("Result node 2");
        setStyle(ImFlow::NodeStyle::brown());
        ImFlow::BaseNode::addIN<int>("A", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addIN<int>("B", 0, ImFlow::ConnectionFilter::SameType());
      }

      void draw() override {
        ImGui::Text("Result: %d", getInVal<int>("A") + getInVal<int>("B"));
      }

      void serialize(nlohmann::json &j) override
      {
      }

      void deserialize(nlohmann::json &j) override
      {
      }
  };


  std::array<std::function<std::shared_ptr<P64Node>(ImFlow::ImNodeFlow &m, const ImVec2&)>, 3> NODE_TABLE = {
    [](ImFlow::ImNodeFlow &m, const ImVec2& pos) { return m.addNode<SimpleSum>(pos); },
    [](ImFlow::ImNodeFlow &m, const ImVec2& pos) { return m.addNode<ResultNode>(pos); },
  };

  struct SavedNode
  {
    uint32_t type{};
    uint32_t uuid{};
    ImVec2 pos{};
    std::string nodeData{};
  };

  struct SavedNodeLink
  {
    uint32_t uuidNodeA;
    std::string linkA;
    uint32_t uuidNodeB;
    std::string linkB;
  };

}

Editor::NodeEditor::NodeEditor()
{
  auto nodeFile = ctx.project->getAssets().getByName("test.p64node");
  auto nodeData = nlohmann::json::parse(
    Utils::FS::loadTextFile(nodeFile->path)
  );

  std::unordered_map<uint64_t, std::shared_ptr<P64Node>> newNodes{};
  for(auto &savedNode : nodeData["nodes"]) {
    uint32_t type = savedNode["type"];
    auto newNode = NODE_TABLE[type](mINF, {});
    newNode->deserialize(savedNode);
    newNode->setPos({savedNode["pos"][0], savedNode["pos"][1]});
    newNode->type = type;
    newNode->uuid = savedNode["uuid"];
    newNodes[newNode->uuid] = newNode;
  }

  for(auto &savedLink : nodeData["links"]) {
    auto nodeAIt = newNodes.find(savedLink["src"]);
    auto nodeBIt = newNodes.find(savedLink["dst"]);
    if(nodeAIt != newNodes.end() && nodeBIt != newNodes.end()) {
      auto pinA = nodeAIt->second->getOuts()[ savedLink["srcPort"] ].get();
      auto pinB = nodeBIt->second->getIns()[ savedLink["dstPort"] ].get();
      if(pinA && pinB) {
        pinA->createLink(pinB);
      }
    }
  }
}

Editor::NodeEditor::~NodeEditor()
{
}

void Editor::NodeEditor::draw()
{
  auto size = ImGui::GetContentRegionAvail();
  size.y -= 32;
  mINF.setSize(size);
  //mINF.getStyle().grid_subdivisions = 10.0f;
  mINF.update();

  if(ImGui::Button("Save"))
  {
    nlohmann::json data{};
    data["nodes"] = nlohmann::json::array();
    for (const auto& [uid, node] : mINF.getNodes()) {
      auto p64Node = (P64Node*)node.get();

      nlohmann::json jNode{};
      jNode["uuid"] = p64Node->uuid;
      jNode["type"] = p64Node->type;
      jNode["pos"] = {p64Node->getPos().x, p64Node->getPos().y};
      p64Node->serialize(jNode);
      data["nodes"].push_back(jNode);
    }

    data["links"] = nlohmann::json::array();
    for (const auto& weakLink : mINF.getLinks()) {
      if (auto link = weakLink.lock()) {
        auto leftPin = link->left();
        auto rightPin = link->right();
        if (leftPin && rightPin) {
          auto leftNode = leftPin->getParent();
          auto rightNode = rightPin->getParent();
          if(leftNode && rightNode) {
            nlohmann::json jLink{};
            jLink["src"] = ((P64Node*)leftNode)->uuid;
            jLink["srcPort"] = 0; // TODO
            jLink["dst"] = ((P64Node*)rightNode)->uuid;
            jLink["dstPort"] = 0; // TODO
            data["links"].push_back(jLink);
          }
        }
      }
    }

    printf("data: %s\n", data.dump(2).c_str());

    auto nodeFile = ctx.project->getAssets().getByName("test.p64node");
    Utils::FS::saveTextFile(nodeFile->path, data.dump(2));
  }
}
