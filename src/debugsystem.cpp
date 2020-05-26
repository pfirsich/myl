#include "debugsystem.hpp"

#include <iostream>
#include <sstream>

#include <imgui.h>
#include <implot.h>

#include "util.hpp"

static const size_t systemInspectorHeight = 400;

DebugSystem::DebugSystem(World& world)
    : world_(world)
{
}

void DebugSystem::update(float dt)
{
    auto& systems = world_.getSystems();

    const auto now = getTime();
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
    for (auto& system : world_.getSystems())
        if (system.name != "_Debug")
            system.enabled = enabled;
}

void DebugSystem::enableForOneFrame()
{
    auto& systems = world_.getSystems();
    for (size_t i = 0; i < systems.size(); ++i) {
        if (!systems[i].enabled) {
            systems[i].enabled = true;
            disableNextFrame_.push_back(i);
        }
    }
}

void DebugSystem::showSystemInspector()
{
    auto& systems = world_.getSystems();
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

void DebugSystem::showEntityInspector()
{
    static EntityId selectedEntity = maxId<EntityId>();

    const auto entities = world_.getEntities();

    const ImGuiIO& io = ImGui::GetIO();
    const size_t width = 500;
    ImGui::SetNextWindowSize(
        ImVec2(width, io.DisplaySize.y - systemInspectorHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - width, 0), ImGuiCond_Once);
    ImGui::Begin("Entity Inspector");

    // Entity List
    {
        ImGui::BeginChild("entity list", ImVec2(200, 0), true);
        for (auto entity : entities) {
            const auto title = "Entity " + std::to_string(static_cast<size_t>(entity));
            if (ImGui::Selectable(title.c_str(), selectedEntity == entity))
                selectedEntity = entity;
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Component Inspector
    if (world_.entityExists(selectedEntity)) {
        const auto entity = selectedEntity;
        const auto& components = world_.getComponents();
        ImGui::BeginChild("components", ImVec2(0, 0), true);

        std::vector<const char*> addOptions;
        for (const auto& component : components) {
            const auto& name = component.getName();
            const auto id = component.getId();
            if (world_.isComponentAllocated(entity, id)) {
                auto ptr = world_.getComponentBuffer(entity, id);
                if (ImGui::CollapsingHeader(getComponentCaption(component, ptr).c_str(),
                        ImGuiTreeNodeFlags_DefaultOpen)) {
                    bool enabled = world_.hasComponent(entity, id);
                    ImGui::Checkbox(("enabled##" + name).c_str(), &enabled);
                    world_.setComponentEnabled(entity, id, enabled);

                    ImGui::SameLine();
                    if (ImGui::Button(("Remove##" + name).c_str(), ImVec2(0, 0))) {
                        world_.removeComponent(entity, id);
                        continue;
                    }
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
            world_.addComponent(entity, world_.getComponentId(addOptions[selected]));
            selected = -1;
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

std::string DebugSystem::getComponentCaption(const Component& component, const void* ptr)
{
    std::stringstream ss;
    ss << static_cast<size_t>(component.getId()) << ": " << component.getName() << " (0x" << ptr
       << ")";
    return ss.str();
}

void DebugSystem::showFieldElement(const Struct::Field& field, void* ptr)
{
    if (field.type->fieldType == FieldType::builtin) {
        auto ft = std::dynamic_pointer_cast<BuiltinFieldType>(field.type);
        switch (ft->type) {
        case BuiltinFieldType::f32:
            ImGui::InputFloat(field.name.c_str(), reinterpret_cast<float*>(ptr));
            break;
        case BuiltinFieldType::vec2:
            ImGui::InputFloat2(field.name.c_str(), reinterpret_cast<float*>(ptr));
            break;
        default:
            ImGui::Text("Unimplemented Builtin Type");
        }
    } else {
        ImGui::Text("Unimplemented Field Type");
    }
}

void DebugSystem::showComponentElements(const Component& component, void* ptr)
{
    ImGui::LabelText("field", "value");
    for (const auto& field : component.getStruct().getFields()) {
        auto fieldPtr = reinterpret_cast<uint8_t*>(ptr) + field.offset;
        showFieldElement(field, fieldPtr);
    }
}
