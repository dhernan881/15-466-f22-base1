#pragma once

#include <glm/glm.hpp>

// Load the color palettes for the game from the saved file.
bool load_palettes(std::vector<glm::u8vec4> *data);

// Write palette data to gen/palettes.sprinfo
bool write_palette_file(std::vector<glm::u8vec4> *data);

// Load the player sprites from its spritesheet.
bool load_player_sprites(std::vector<glm::u8vec4> *data);

// Write player sprite data to gen/player.sprinfo
bool write_player_sprites_file(std::vector<glm::u8vec4> *data);
