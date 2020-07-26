#include "batch.hpp"

namespace {
static constexpr std::string_view vertSource = R"(
    #version 330 core

    uniform mat4 transform;

    layout (location = 0) in vec2 aPosition;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec4 aColor;

    out vec2 texCoord;
    out vec4 color;

    void main() {
        texCoord = aTexCoord;
        color = aColor;
        gl_Position = transform * vec4(aPosition, 0.0, 1.0);
    }
)";

static constexpr std::string_view fragSource = R"(
    #version 330 core

    uniform sampler2D tex;

    in vec2 texCoord;
    in vec4 color;

    out vec4 fragColor;

    void main() {
        fragColor = color; //texture2D(tex, texCoord);
    }
)";

const glw::ShaderProgram& getShader()
{
    static const auto shader = glwx::makeShaderProgram(vertSource, fragSource).value();
    return shader;
}

glw::VertexFormat makeVertexFormat()
{
    glw::VertexFormat fmt;
    fmt.add(0, 2, glw::AttributeType::F32);
    fmt.add(1, 2, glw::AttributeType::F32);
    fmt.add(2, 4, glw::AttributeType::F32);
    return fmt;
}

const glw::VertexFormat& getVertexFormat()
{
    static glw::VertexFormat fmt = makeVertexFormat();
    return fmt;
}
}

namespace myl {
Batch::Batch(size_t vertexCount, size_t indexCount)
    : primitive_(glw::DrawMode::Triangles)
    , transform_(glm::mat4(1.0f))
{
    vertices_.reserve(vertexCount);
    vertexBuffer_.data(
        glw::Buffer::Target::Array, glw::Buffer::UsageHint::StreamDraw, nullptr, vertexCount);
    indices_.reserve(indexCount);
    indexBuffer_.data(
        glw::Buffer::Target::ElementArray, glw::Buffer::UsageHint::StreamDraw, nullptr, indexCount);

    primitive_.addVertexBuffer(vertexBuffer_, getVertexFormat());
    primitive_.setIndexBuffer(indexBuffer_, glw::getIndexType(indexCount));
}

glm::mat4 Batch::getTransform() const
{
    return transform_;
}

void Batch::setTransform(const glm::mat4& transform)
{
    transform_ = transform;
}

Batch::IndexType Batch::addVertex(
    const glm::vec2& position, const glm::vec2& texCoord, const glm::vec4& color)
{
    assert(hasCapacity(1, 0));
    vertices_.push_back(Vertex { position, texCoord, color });
    const auto index = static_cast<IndexType>(vertices_.size() - 1);
    assert(index <= std::numeric_limits<IndexType>::max());
    return index;
}

void Batch::addIndex(IndexType index)
{
    assert(hasCapacity(0, 1));
    indices_.push_back(index);
}

// For some reason clang doesn't want me to give th function below a default argument of
// TexRegion {}, so I use this overload
void Batch::addRectangle(
    const components::Transform& transform, const glm::vec4& color, const glm::vec2& size)
{
    addRectangle(transform, color, size, TexRegion {});
}

void Batch::addRectangle(const components::Transform& transform, const glm::vec4& color,
    const glm::vec2& size, const TexRegion& tc)
{
    if (!hasCapacity(4, 6))
        flush();

    const auto tl = addVertex(transform.apply(glm::vec2(0.0f, 0.0f)), tc.offset, color);
    const auto tr = addVertex(
        transform.apply(glm::vec2(size.x, 0.0f)), tc.offset + glm::vec2(tc.size.x, 0.0f), color);
    const auto br = addVertex(transform.apply(size), tc.offset + tc.size, color);
    const auto bl = addVertex(
        transform.apply(glm::vec2(0.0f, size.y)), tc.offset + glm::vec2(0.0f, tc.size.y), color);

    addIndex(tl);
    addIndex(bl);
    addIndex(tr);

    addIndex(bl);
    addIndex(br);
    addIndex(tr);
}

void Batch::addCircle(
    const components::Transform& transform, const glm::vec4& color, float radius, size_t segments)
{
    const float angleSegment = 2.0f * M_PI / segments;
    const auto center = addVertex(transform.apply(glm::vec2(0.0f)), glm::vec2(0.5f), color);
    for (size_t i = 0; i < segments + 1; ++i) {
        const auto x = std::cos(angleSegment * i);
        const auto y = std::sin(angleSegment * i);
        const auto idx = addVertex(
            transform.apply(radius * glm::vec2(x, y)), glm::vec2(x, y) * 0.5f + 0.5f, color);
        if (i > 0) {
            addIndex(center);
            addIndex(idx);
            addIndex(idx - 1);
        }
    }
}

const std::vector<Batch::Vertex>& Batch::getVertices() const
{
    return vertices_;
}

const std::vector<Batch::IndexType>& Batch::getIndices() const
{
    return indices_;
}

void Batch::clear()
{
    vertices_.clear();
    indices_.clear();
}

bool Batch::hasCapacity(size_t vertices, size_t indices) const
{
    if ((vertices_.size() + vertices) * sizeof(Vertex) > vertexBuffer_.getSize())
        return false;
    if ((indices_.size() + indices) * sizeof(IndexType) > indexBuffer_.getSize())
        return false;
    return true;
}

void Batch::flush()
{
    getShader().bind();
    getShader().setUniform("transform", transform_);
    vertexBuffer_.subData(glw::Buffer::Target::Array, vertices_.data(), vertices_.size());
    indexBuffer_.subData(glw::Buffer::Target::ElementArray, indices_.data(), indices_.size());
    primitive_.draw(0, indices_.size());
    clear();
    glw::State::instance().bindVao(0);
    glw::State::instance().bindBuffer(GL_ARRAY_BUFFER, 0);
    glw::State::instance().bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glw::State::instance().unbindShader();
}
}
