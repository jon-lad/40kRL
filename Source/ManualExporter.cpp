// Source/ManualExporter.cpp
#include "ManualExporter.h"
#include "HelpContent.h"

#include <fstream>
#include <string>
#include <string_view>

namespace ManualExporter {

std::string generate(std::string_view version) {
    std::string output;

    // Title header
    output += "40kRL MANUAL\n";
    output += "Version ";
    output += version;
    output += "\n\n";

    // Format each section
    auto sections = HelpContent::allSections();
    for (const auto& section : sections) {
        // Section title
        output += section.title;
        output += "\n";

        // Dashed underline of equal length to title
        output.append(section.title.size(), '-');
        output += "\n";

        // Indented entries: "  key  description"
        for (const auto& entry : section.entries) {
            output += "  ";
            output += entry.key;
            output += "  ";
            output += entry.description;
            output += "\n";
        }

        // Blank line after section
        output += "\n";
    }

    return output;
}

bool writeToFile(std::string_view path, std::string_view version) {
    std::string content = generate(version);

    std::ofstream file(std::string(path), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    file << content;
    return file.good();
}

} // namespace ManualExporter
