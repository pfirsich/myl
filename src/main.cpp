#include <fstream> //required for toml::parse_file()
#include <iostream>
#include <set>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <toml++/toml.h>

#include "component.hpp"
#include "fieldtype.hpp"

#include <variant>

struct S {
    uint64_t id;
    std::string name;
};

struct TypeCache {
    std::unordered_map<std::string, std::shared_ptr<EnumFieldType>> enums;
    std::unordered_map<std::string, std::shared_ptr<StructFieldType>> structs;
};

std::optional<std::string> getBracketType(std::string_view str)
{
    const auto openBracket = str.find_last_of('[');
    if (openBracket == std::string::npos)
        return std::nullopt;
    if (str[str.length() - 1] != ']') // type string is malformed
        return std::nullopt;
    return std::string(str.substr(openBracket + 1, str.length() - openBracket - 2));
}

bool isDigit(std::string_view str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

std::shared_ptr<FieldType> parseType(const std::string& typeStr, const TypeCache& cache)
{
    const auto bracketType = getBracketType(typeStr);
    if (bracketType) {
        const auto baseType = typeStr.substr(0, typeStr.length() - bracketType->length() - 2);
        if (bracketType->empty()) {
            return std::make_shared<VectorFieldType>(parseType(baseType, cache));
        } else if (isDigit(*bracketType)) {
            const auto arraySize = boost::lexical_cast<size_t>(*bracketType);
            return std::make_shared<ArrayFieldType>(parseType(baseType, cache), arraySize);
        } else {
            return std::make_shared<MapFieldType>(
                parseType(*bracketType, cache), parseType(baseType, cache));
        }
    }

    const auto builtinIt = BuiltinFieldType::typeFromString.find(typeStr);
    if (builtinIt != BuiltinFieldType::typeFromString.end())
        return std::make_shared<BuiltinFieldType>(builtinIt->second);

    const auto enumIt = cache.enums.find(typeStr);
    if (enumIt != cache.enums.end())
        return enumIt->second;

    const auto structIt = cache.structs.find(typeStr);
    if (structIt != cache.structs.end())
        return structIt->second;

    return std::make_shared<PlaceHolderFieldType>(typeStr);
}

std::shared_ptr<StructFieldType> parseStruct(const toml::table& fields, const TypeCache& cache)
{

    const auto structType = std::make_shared<StructFieldType>();
    for (const auto&& [name, type] : *fields.as_table()) {
        const auto typeStr = type.as_string()->get();
        structType->fields.push_back(std::make_pair(name, parseType(typeStr, cache)));
    }
    return structType;
}

std::unordered_map<std::string, Component> loadComponentFromFile(std::string_view path)
{
    // If the file is malformed, this will do weird shit. I don't care, go fuck yourself.
    std::unordered_map<std::string, Component> components;

    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& err) {
        std::cerr << "Error parsing file '" << *err.source().path << "':\n"
                  << err.description() << "\n  (" << err.source().begin << ")\n";
        return components;
    }

    TypeCache cache;

    for (const auto&& [enumName, enum_] : *tbl["enums"].as_table()) {
        const auto& enumTable = *enum_.as_table();
        std::cout << "# Enum: " << enumName << std::endl;
        std::vector<std::string> values;
        for (const auto& value : *enumTable["values"].as_array()) {
            values.push_back(value.as_string()->get());
        }
        cache.enums.emplace(enumName, std::make_shared<EnumFieldType>(values));
    }

    std::set<std::string> componentNames;
    for (const auto&& [name, struct_] : *tbl["structs"].as_table()) {
        if (name == "components") {
            for (const auto& value : *struct_.as_array()) {
                componentNames.insert(value.as_string()->get());
            }
            continue;
        }

        const auto& fields = *struct_.as_table();
        const auto structType = parseStruct(fields, cache);
        cache.structs.emplace(name, structType);
    }

    // Replace placeholders
    for (auto&& [name, structType] : cache.structs) {
        for (auto&& [otherName, otherStructType] : cache.structs) {
            if (name != otherName)
                structType->replacePlaceholder(otherName, otherStructType);
        }
    }

    for (auto&& [name, structType] : cache.structs) {
        if (componentNames.count(name))
            std::cout << "# Component: " << name << std::endl;
        else
            std::cout << "# Struct: " << name << std::endl;
        std::cout << structType->asString() << std::endl;
    }

    return components;
}

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