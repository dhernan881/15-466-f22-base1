#pragma once

#include <string>

#include <glm/glm.hpp>

// Write palette data to gen/palettes.sprinfo
bool write_palette_file(std::vector<glm::u8vec4> *data);

// Generic function to load a spritesheet
bool load_sprites(std::vector<glm::u8vec4> *data, std::string path,
    uint expected_x, uint expected_y);

// Generic function to write sprites to a .sprinfo file.
// Sprites should be 8 wide and will be mapped to a number in [0-3] based on
// their r component. They should be in greyscale where the r component
// corresponds to the index of the color in the desired color palette to use.
bool write_sprites_file(std::vector<glm::u8vec4> *data, std::string path);
