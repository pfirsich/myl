#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "ecs.hpp"
#include "ringbuffer.hpp"

class DebugSystem : public myl::RegisteredSystem<DebugSystem> {
public:
    inline static const std::string name = "_Debug";

    void update(float dt);

private:
    void showMiniMenu();

    void setSystemsEnabled(bool enabled);
    void enableForOneFrame();
    void showSystemInspector();

    void showEntityInspector();

    static std::string getComponentCaption(const myl::Component& component, const void* ptr);
    static void showFieldElement(
        const std::string& name, std::shared_ptr<myl::FieldType> fieldType, void* ptr);
    static void showComponentElements(const myl::Component& component, void* ptr);

    bool showEntityInspector_ = false;
    bool showSystemInspector_ = false;
    bool showImGuiDemoWindow_ = false;
    bool showImPlotDemoWindow_ = false;
    std::vector<size_t> disableNextFrame_;
    size_t systemFrameTimeSampleCount_ = 1024;
    std::vector<RingBuffer<glm::vec2>> systemFrameTimes_;
    glm::vec2 timeRange_;
};
