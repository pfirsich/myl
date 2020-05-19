#include "componentfile.hpp"
#include "struct.hpp"

struct S {
    uint64_t id;
    std::string name;
};

int main(int, char**)
{
    const auto components = loadComponentFromFile("component.toml");

    StructBuilder sb;
    sb.addField<uint64_t>("id");
    sb.addField<std::string>("name");

    const auto s = sb.build();
    const auto sbuffer = s.allocate();
    for (const auto& field : s.getFields()) {
    }

    sb.addField<decltype(S::id)>("id");
}