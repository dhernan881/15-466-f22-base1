#include <iostream>
#include <vector>
#include <glm/gtx/string_cast.hpp>

#include "asset_libs.hpp"
#include "data_path.hpp"

int main(int argc, char **argv) {
    std::vector<glm::u8vec4> palette_data;
    // Load palettes file.
    if (!load_sprites(&palette_data, data_path("assets/palettes.png"),
            4, 8)) {
        std::cerr << "Error loading palettes." << std::endl;
        return 1;
    }
    std::cout << "Palettes loaded successfully!" << std::endl;
    // Write palettes to a .sprinfo file.
    if (!write_palette_file(&palette_data)) {
        std::cerr << "Error writing palette data to .sprinfo file." << std::endl;
        return 1;
    }
    std::cout << "Pallets successfully compiled to .sprinfo file!" << std::endl;
    
    // Load the player sprites.
    std::vector<glm::u8vec4> player_sprite_data;
    if (!load_sprites(&player_sprite_data, data_path("assets/spritesheets/player.png"),
            8, 8 * 16)) {
        std::cerr << "Error loading player sprites." << std::endl;
        return 1;
    }
    std::cout << "Player sprites loaded successfully!" << std::endl;
    // Write the player sprites to a .sprinfo file.
    if (!write_sprites_file(&player_sprite_data, data_path("gen/player.sprinfo"))) {
        std::cerr << "Error writing player sprite data to .sprinfo file." << std::endl;
        return 1;
    }
    std::cout << "Player sprites successfully compiled to .sprinfo file!" << std::endl;

    // Load the water detail sprites.
    std::vector<glm::u8vec4> water_sprite_data;
    if (!load_sprites(&water_sprite_data, data_path("assets/spritesheets/water.png"),
            8, 8 * 8)) {
        std::cerr << "Error loading water sprites." << std::endl;
        return 1;
    }
    std::cout << "Water sprites loaded successfully!" << std::endl;
    // Write the water sprites to a .sprinfo file.
    if (!write_sprites_file(&water_sprite_data, data_path("gen/water.sprinfo"))) {
        std::cerr << "Error writing water sprite data to .sprinfo file." << std::endl;
        return 1;
    }
    return 0;
}
