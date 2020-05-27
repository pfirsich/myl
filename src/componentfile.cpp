#include "componentfile.hpp"

#include <fstream> //required for toml::parse_file()
#include <iostream>
#include <optional>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <toml++/toml.h>

#include "fieldtype.hpp"

namespace myl {

struct StructData {
    StructType structType;
    std::string name;
    bool isComponent;
};

struct ComponentFileData {
    vector_map<std::string, EnumType> enums;
    vector_map<std::string, StructData> structs;
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

std::shared_ptr<FieldType> parseType(const std::string& typeStr, const ComponentFileData& cache)
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
        return std::make_shared<EnumFieldType>(enumIt->first);

    const auto structIt = cache.structs.find(typeStr);
    if (structIt != cache.structs.end())
        return std::make_shared<StructFieldType>(structIt->first);

    return std::make_shared<ErrorFieldType>(typeStr);
}

StructType parseStruct(const toml::array& fields, const ComponentFileData& cache)
{
    StructType structType;
    for (const auto& field : fields) {
        const auto& fieldTable = *field.as_table();
        const auto name = fieldTable["name"].as_string()->get();
        const auto type = fieldTable["type"].as_string()->get();
        structType.fields.emplace_back(std::make_pair(name, parseType(type, cache)));
    }
    return structType;
}

bool hasErrorType(StructType& structType)
{
    for (auto& [name, field] : structType.fields) {
        bool has = false;
        traverse(
            [&has](std::shared_ptr<FieldType>& fieldType) {
                has = has || fieldType->fieldType == FieldType::error;
            },
            field);
        if (has)
            return true;
    }
    return false;
}

ComponentFileData loadComponentFromFile(std::string_view path)
{
    // If the file is malformed, this will do weird shit. I don't care, go fuck yourself.
    ComponentFileData data;

    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& err) {
        std::cerr << "Error parsing file '" << *err.source().path << "':\n"
                  << err.description() << "\n  (" << err.source().begin << ")\n";
        return data;
    }

    if (tbl["enums"]) {
        for (const auto& enum_ : *tbl["enums"].as_array()) {
            const auto& enumTable = *enum_.as_table();
            const auto enumName = enumTable["name"].as_string()->get();
            std::cout << "# Enum: " << enumName << std::endl;
            std::vector<std::string> values;
            for (const auto& value : *enumTable["values"].as_array()) {
                values.push_back(value.as_string()->get());
            }
            EnumType enumType { values };
            std::cout << enumType.asString() << std::endl;
            data.enums.insert(enumName, enumType);
        }
    }

    for (const auto& struct_ : *tbl["structs"].as_array()) {
        const auto& structTable = *struct_.as_table();
        const auto name = structTable["name"].as_string()->get();

        auto structType = parseStruct(*structTable["fields"].as_array(), data);
        if (hasErrorType(structType)) {
            std::cerr << "Type '" << name << "' includes error types!" << std::endl;
        }

        const bool isComponent = structTable["component"].value_or(false);
        data.structs.insert(name, StructData { structType, name, isComponent });

        if (isComponent)
            std::cout << "# Component: " << name << std::endl;
        else
            std::cout << "# Struct: " << name << std::endl;
        std::cout << structType.asString() << std::endl;
    }

    return data;
}

void loadComponentsFromFile(World& world, std::string_view path)
{
    const auto componentData = loadComponentFromFile(path);
    for (const auto& [name, component] : componentData.structs) {
        if (!component.isComponent)
            continue;

        StructBuilder sb;
        for (const auto& [fieldName, fieldType] : component.structType.fields) {
            sb.addField(fieldName, fieldType);
        }
        world.registerComponent(name, sb.build());
    }
}

void loadComponentsFromFile(std::string_view path)
{
    loadComponentsFromFile(getDefaultWorld(), path);
}

}