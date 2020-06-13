#include "debug.hpp"

#include <iostream>
#include <sstream>

#include <imgui.h>
#include <implot.h>
#include <misc/cpp/imgui_stdlib.h>

#include "glmformat.hpp"
#include "modules/tweak.hpp"
#include "util.hpp"

static const size_t systemInspectorHeight = 400;

void DebugSystem::update(float /*dt*/)
{
    auto& systems = myl::getSystems();

    const auto now = myl::getTime();
    timeRange_.x = now;
    timeRange_.y = now;
    for (size_t i = 0; i < systems.size(); ++i) {
        if (systems[i].enabled) {
            if (i >= systemFrameTimes_.size())
                systemFrameTimes_.emplace_back(systemFrameTimeSampleCount_);
            systemFrameTimes_[i].push(glm::vec2(now, systems[i].lastDuration * 1000.0f));
            timeRange_.x = std::min(timeRange_.x, systemFrameTimes_[i][0].x);
        }
    }

    if (!disableNextFrame_.empty()) {
        for (const auto idx : disableNextFrame_)
            systems[idx].enabled = false;
        disableNextFrame_.clear();
    }

    showMiniMenu();

    if (showEntityInspector_)
        showEntityInspector();

    if (showSystemInspector_)
        showSystemInspector();

    if (showTweakInspector_)
        showTweakInspector();

    if (showImGuiDemoWindow_)
        ImGui::ShowDemoWindow(nullptr);

    if (showImPlotDemoWindow_)
        ImPlot::ShowDemoWindow(nullptr);
}

void DebugSystem::showMiniMenu()
{
    enum class Corner { TopLeft, TopRight, BottomLeft, BottomRight };
    const float margin = 10.0f;
    static Corner corner = Corner::TopLeft;

    const ImGuiIO& io = ImGui::GetIO();

    const bool left = corner == Corner::TopLeft || corner == Corner::BottomLeft;
    const float x = left ? margin : io.DisplaySize.x - margin;
    const bool top = corner == Corner::TopLeft || corner == Corner::TopRight;
    const float y = top ? margin : io.DisplaySize.y - margin;
    ImVec2 pivot = ImVec2(left ? 0.0f : 1.0f, top ? 0.0f : 0.0f);
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always, pivot);

    ImGui::SetNextWindowBgAlpha(0.35f);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImGui::Begin("Debug Mode", nullptr, flags);

    if (ImGui::Button("Entity Inspector"))
        showEntityInspector_ = !showEntityInspector_;

    if (ImGui::Button("System Inspector"))
        showSystemInspector_ = !showSystemInspector_;

    if (ImGui::Button("Tweak Inspector"))
        showTweakInspector_ = !showTweakInspector_;

    if (ImGui::Button("System ImGui Demo Window"))
        showImGuiDemoWindow_ = !showImGuiDemoWindow_;

    if (ImGui::Button("System ImPlot Demo Window"))
        showImPlotDemoWindow_ = !showImPlotDemoWindow_;

    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Top Left", NULL, corner == Corner::TopLeft))
            corner = Corner::TopLeft;
        if (ImGui::MenuItem("Top Right", NULL, corner == Corner::TopRight))
            corner = Corner::TopRight;
        if (ImGui::MenuItem("Bottom Left", NULL, corner == Corner::BottomLeft))
            corner = Corner::BottomLeft;
        if (ImGui::MenuItem("Bottom Right", NULL, corner == Corner::BottomRight))
            corner = Corner::BottomRight;
        ImGui::EndPopup();
    }
    ImGui::End();
}

void DebugSystem::setSystemsEnabled(bool enabled)
{
    for (auto& system : myl::getSystems())
        if (system.name != "_Debug")
            system.enabled = enabled;
}

void DebugSystem::enableForOneFrame()
{
    auto& systems = myl::getSystems();
    for (size_t i = 0; i < systems.size(); ++i) {
        if (!systems[i].enabled) {
            systems[i].enabled = true;
            disableNextFrame_.push_back(i);
        }
    }
}

void DebugSystem::showSystemInspector()
{
    auto& systems = myl::getSystems();
    static size_t selectedSystem = systems.size() + 1;

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, systemInspectorHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - systemInspectorHeight), ImGuiCond_Once);
    ImGui::Begin("System Inspector");
    // System List
    {
        ImGui::BeginChild("system list", ImVec2(250, 0), true);
        for (size_t i = 0; i < systems.size(); ++i) {
            if (systems[i].name != "_Debug") {
                ImGui::Checkbox(("##" + systems[i].name).c_str(), &systems[i].enabled);
                ImGui::SameLine();
            }
            if (ImGui::Selectable(systems[i].name.c_str(), selectedSystem == i))
                selectedSystem = i;
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    ImGui::BeginChild("systemgraph");

    // System Controls
    {
        if (ImGui::Button("Disable All"))
            setSystemsEnabled(false);
        ImGui::SameLine();
        if (ImGui::Button("Enable All"))
            setSystemsEnabled(true);
        ImGui::SameLine();
        if (ImGui::Button("Enable All for One Frame"))
            enableForOneFrame();
    }
    ImGui::Separator();

    // Plots
    {
        if (ImGui::Button("Clear Graphs")) {
            for (size_t i = 0; i < systems.size(); ++i) {
                systemFrameTimes_[i].clear();
            }
        }

        ImPlot::SetNextPlotLimitsX(timeRange_.x, timeRange_.y, ImGuiCond_Always);
        if (ImPlot::BeginPlot("##SystemTimes", "time [s]", "duration [ms]", ImVec2(-1, -1))) {
            for (size_t i = 0; i < systems.size(); ++i) {
                ImPlot::PlotLine(systems[i].name.c_str(), &systemFrameTimes_[i].getData()->x,
                    &systemFrameTimes_[i].getData()->y, systemFrameTimes_[i].getSize(),
                    systemFrameTimes_[i].getIndex(0), 8);
            }
            ImPlot::EndPlot();
        }
    }

    ImGui::EndChild();

    ImGui::End();
}

std::string getEntityName(myl::EntityId id)
{
    static auto cName = myl::getComponentId("Name");
    if (myl::hasComponent(id, cName))
        // This is a hack. The component itself is not a String, but the only member is.
        return myl::getComponent<myl::String>(id, cName)->str();
    return "Entity " + id.toString();
}

void DebugSystem::showEntityInspector()
{
    static auto selectedEntity = myl::maxId<myl::EntityId>();

    const auto entities = myl::getEntities();

    const ImGuiIO& io = ImGui::GetIO();
    const size_t width = 500;
    ImGui::SetNextWindowSize(
        ImVec2(width, io.DisplaySize.y - systemInspectorHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - width, 0), ImGuiCond_Once);
    ImGui::Begin("Entity Inspector");

    // Entity List
    {
        ImGui::BeginChild("entity list", ImVec2(200, 0), true);

        if (ImGui::Button("Create Entity"))
            myl::newEntity();
        ImGui::Separator();

        for (auto entity : entities) {
            if (ImGui::Selectable(getEntityName(entity).c_str(), selectedEntity == entity))
                selectedEntity = entity;
        }

        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Component Inspector
    if (myl::entityExists(selectedEntity)) {
        const auto entity = selectedEntity;
        const auto& components = myl::getComponents();
        ImGui::BeginChild("entity view", ImVec2(0, 0), true);

        ImGui::Text("ID: %zu", static_cast<size_t>(selectedEntity));

        ImGui::Separator();

        std::vector<const char*> addOptions;
        for (const auto& component : components) {
            const auto& name = component.getName();
            const auto id = component.getId();
            if (myl::isComponentAllocated(entity, id)) {
                auto ptr = myl::getComponentBuffer(entity, id);
                if (ImGui::CollapsingHeader(getComponentCaption(component, ptr).c_str(),
                        ImGuiTreeNodeFlags_DefaultOpen)) {
                    bool enabled = myl::hasComponent(entity, id);
                    ImGui::Checkbox(("enabled##" + name).c_str(), &enabled);
                    myl::setComponentEnabled(entity, id, enabled);

                    ImGui::SameLine();
                    if (ImGui::Button(("Remove##" + name).c_str(), ImVec2(0, 0))) {
                        myl::removeComponent(entity, id);
                        continue;
                    }
                    ImGui::Separator();
                    showComponentElements(component, ptr);
                }
            } else {
                addOptions.push_back(name.c_str());
            }
        }

        ImGui::Separator();

        static int selected = -1;
        ImGui::Combo("Component", &selected, &addOptions[0], addOptions.size());

        if (ImGui::Button("Add Component###Button", ImVec2(-1, 0)) && selected != -1) {
            myl::addComponent(entity, myl::getComponentId(addOptions[selected]));
            selected = -1;
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void DebugSystem::showTweakInspector()
{
    using namespace myl::modules::tweak;

    const ImGuiIO& io = ImGui::GetIO();
    const size_t width = 500;
    ImGui::SetNextWindowSize(
        ImVec2(width, io.DisplaySize.y - systemInspectorHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - width, 0), ImGuiCond_Once);
    ImGui::Begin("Tweak Inspector");

    ImGui::LabelText("Value", "name");

    for (const auto& [name, index] : getTweaks()) {
        switch (static_cast<ValueType>(index)) {
        case ValueType::Int: {
            const auto init = getInitial<int>(name);
            const auto title = fmt::format("{} ({})", name, init);
            auto val = get<int>(name);
            ImGui::InputInt(title.c_str(), &val);
            set(name, val);
            break;
        }
        case ValueType::Float: {
            const auto init = getInitial<float>(name);
            const auto title = fmt::format("{} ({})", name, init);
            auto val = get<float>(name);
            ImGui::InputFloat(title.c_str(), &val);
            set(name, val);
            break;
        }
        case ValueType::Vec2: {
            const auto init = getInitial<glm::vec2>(name);
            const auto title = fmt::format("{} {}", name, init);
            auto val = get<glm::vec2>(name);
            ImGui::InputFloat2(title.c_str(), &val.x);
            set(name, val);
            break;
        }
        case ValueType::Vec3: {
            const auto init = getInitial<glm::vec3>(name);
            const auto title = fmt::format("{} {}", name, init);
            auto val = get<glm::vec3>(name);
            ImGui::InputFloat3(title.c_str(), &val.x);
            set(name, val);
            break;
        }
        case ValueType::Vec4: {
            const auto init = getInitial<glm::vec4>(name);
            const auto title = fmt::format("{} {}", name, init);
            auto val = get<glm::vec4>(name);
            ImGui::InputFloat4(title.c_str(), &val.x);
            set(name, val);
            break;
        }
        case ValueType::String: {
            const auto init = getInitial<std::string>(name);
            const auto title = fmt::format("{} (\"{}\")", name, init);
            auto val = get<std::string>(name);
            ImGui::InputText(title.c_str(), &val);
            set(name, val);
            break;
        }
        case ValueType::Color: {
            const auto init = getInitial<myl::Color>(name);
            const auto title = fmt::format("{} {}", name, init);
            auto val = get<myl::Color>(name);
            ImGui::ColorEdit4(name.c_str(), &val.r,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf
                    | ImGuiColorEditFlags_Float);
            set(name, val);
            break;
        }
        case ValueType::Last:
            break;
        }
    }

    ImGui::End();
}

std::string DebugSystem::getComponentCaption(const myl::Component& component, const void* ptr)
{
    std::stringstream ss;
    ss << static_cast<size_t>(component.getId()) << ": " << component.getName() << " (0x" << ptr
       << ")";
    return ss.str();
}

void DebugSystem::showFieldElement(
    const std::string& name, std::shared_ptr<myl::FieldType> fieldType, void* ptr)
{
    switch (fieldType->fieldType) {
    case myl::FieldType::Builtin: {
        auto ft = std::dynamic_pointer_cast<myl::PrimitiveFieldType>(fieldType);
        switch (ft->type) {
        case myl::PrimitiveFieldType::Bool:
            ImGui::Checkbox(name.c_str(), reinterpret_cast<bool*>(ptr));
            break;
        case myl::PrimitiveFieldType::U8:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_U8, ptr, nullptr, nullptr, "%u");
            break;
        case myl::PrimitiveFieldType::I8:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, ptr, nullptr, nullptr, "%d");
            break;
        case myl::PrimitiveFieldType::U16:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, ptr, nullptr, nullptr, "%u");
            break;
        case myl::PrimitiveFieldType::I16:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_S16, ptr, nullptr, nullptr, "%d");
            break;
        case myl::PrimitiveFieldType::U32:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, ptr, nullptr, nullptr, "%u");
            break;
        case myl::PrimitiveFieldType::I32:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_S32, ptr, nullptr, nullptr, "%d");
            break;
        case myl::PrimitiveFieldType::U64:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_U64, ptr, nullptr);
            break;
        case myl::PrimitiveFieldType::I64:
            ImGui::InputScalar(name.c_str(), ImGuiDataType_S64, ptr, nullptr);
            break;
        case myl::PrimitiveFieldType::F32:
            ImGui::InputFloat(name.c_str(), reinterpret_cast<float*>(ptr));
            break;
        case myl::PrimitiveFieldType::Vec2:
            ImGui::InputFloat2(name.c_str(), reinterpret_cast<float*>(ptr));
            break;
        case myl::PrimitiveFieldType::Vec3:
            ImGui::InputFloat3(name.c_str(), reinterpret_cast<float*>(ptr));
            break;
        case myl::PrimitiveFieldType::Vec4:
            ImGui::InputFloat4(name.c_str(), reinterpret_cast<float*>(ptr));
            break;
        case myl::PrimitiveFieldType::Color:
            ImGui::ColorEdit4(name.c_str(), reinterpret_cast<float*>(ptr),
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf
                    | ImGuiColorEditFlags_Float);
            break;
        default:
            ImGui::Text("Unimplemented Builtin Type");
            break;
        }
        break;
    }
    case myl::FieldType::String: {
        std::string text = reinterpret_cast<myl::String*>(ptr)->str();
        ImGui::InputText(name.c_str(), &text);
        reinterpret_cast<myl::String*>(ptr)->assign(text);
        break;
    }
    case myl::FieldType::Array: {
        auto ft = std::dynamic_pointer_cast<myl::ArrayFieldType>(fieldType);
        if (ImGui::TreeNode(name.c_str())) {
            for (size_t i = 0; i < ft->size; ++i) {
                auto elemPtr = reinterpret_cast<uint8_t*>(ptr) + i * ft->elementType->getSize();
                showFieldElement(std::to_string(i), ft->elementType, elemPtr);
            }
            ImGui::TreePop();
        }
        break;
    }
    case myl::FieldType::Vector: {
        auto ft = std::dynamic_pointer_cast<myl::VectorFieldType>(fieldType);
        if (ImGui::TreeNode(name.c_str())) {
            auto& vec = *reinterpret_cast<myl::Vector*>(ptr);
            if (ImGui::Button("Push"))
                vec.pushBack();
            ImGui::SameLine();
            if (ImGui::Button("Pop"))
                vec.popBack();
            ImGui::Separator();

            for (size_t i = 0; i < vec.getSize(); ++i) {
                auto elemPtr = reinterpret_cast<uint8_t*>(vec.getData()) + i * vec.getElementSize();
                showFieldElement(std::to_string(i), ft->elementType, elemPtr);
            }
            ImGui::TreePop();
        }
        break;
    }
    default:
        ImGui::Text("Unimplemented Field Type");
    }
}

void DebugSystem::showComponentElements(const myl::Component& component, void* ptr)
{
    for (const auto& field : component.getStruct().getFields()) {
        auto fieldPtr = reinterpret_cast<uint8_t*>(ptr) + field.offset;
        showFieldElement(field.name, field.type, fieldPtr);
    }
}
