// Headers/ManualExporter.h
#pragma once
#include <string>
#include <string_view>

namespace ManualExporter {

// Generates the full MANUAL.txt content as a string.
// version: e.g. "0.3.0"
std::string generate(std::string_view version);

// Writes generate() output to the given file path. Returns true on success.
bool writeToFile(std::string_view path, std::string_view version);

} // namespace ManualExporter
