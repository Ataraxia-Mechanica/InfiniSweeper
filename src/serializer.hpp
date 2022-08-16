#pragma once

#include <string>

class Level;
namespace Serializer {
// completed_levels is useful for level selection menu
void Load(std::string name, Level& level, int completed_levels = 0);
};  // namespace Serializer
