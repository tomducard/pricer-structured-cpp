// Small helpers to stringify/parse numeric vectors from the GUI text fields.
#pragma once

#include <string>
#include <vector>

std::string vectorToString(const std::vector<double>& values);
std::vector<double> parseTimesList(const std::string& text,
                                   const std::vector<double>& fallback);
