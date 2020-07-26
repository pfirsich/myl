#pragma once

#include <glm/glm.hpp>
#include <glw.hpp>
#include <glwx.hpp>

#include "components.hpp"

namespace myl {
class Batch {
public:
    using IndexType = uint16_t;

    struct Vertex {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 color;
    };

    struct TexRegion {
        glm::vec2 offset { 0.0f, 0.0f };
        glm::vec2 size { 1.0f, 1.0f };
    };

    Batch(size_t vertexCount, size_t indexCount);

    glm::mat4 getTransform() const;
    void setTransform(const glm::mat4& transform);

    IndexType addVertex(
        const glm::vec2& position, const glm::vec2& texCoord, const glm::vec4& color);

    void addIndex(IndexType index);

    // For some reason clang doesn't want me to give th function below a default argument of
    // TexRegion {}, so I use this overload
    void addRectangle(
        const components::Transform& transform, const glm::vec4& color, const glm::vec2& size);

    void addRectangle(const components::Transform& transform, const glm::vec4& color,
        const glm::vec2& size, const TexRegion& tc);

    void addCircle(const components::Transform& transform, const glm::vec4& color, float radius,
        size_t segments = 32);

    const std::vector<Vertex>& getVertices() const;
    const std::vector<IndexType>& getIndices() const;

    void clear();

    bool hasCapacity(size_t vertices, size_t indices) const;

    void flush();

private:
    glwx::Primitive primitive_;
    glm::mat4 transform_;
    glw::Buffer vertexBuffer_;
    glw::Buffer indexBuffer_;
    std::vector<Vertex> vertices_;
    std::vector<IndexType> indices_;
};
}
