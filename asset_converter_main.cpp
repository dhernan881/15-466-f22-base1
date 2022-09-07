#include <iostream>
#include <vector>
#include <glm/gtx/string_cast.hpp>

#include "asset_libs.hpp"

int main(int argc, char **argv) {
    std::vector<glm::u8vec4> palette_data;
    // Load palettes file
    if (!load_palettes(&palette_data)) {
        std::cerr << "Error loading palettes." << std::endl;
        return 1;
    }
    std::cout << "Palettes loaded successfully!" << std::endl;
    // for (auto &color : palette_data) {
    //     std::cout << glm::to_string(color) << "\n";
    // }
    // Write palettes here? Or somewhere later?

    if (!write_palette_file(&palette_data)) {
        std::cerr << "Error writing palette data to .sprinfo file." << std::endl;
        return 1;
    }
    std::cout << "Pallets successfully compiled to .sprinfo file!" << std::endl;
    
    // Load the player sprites.
    std::vector<glm::u8vec4> player_sprite_data;
    if (!load_player_sprites(&player_sprite_data)) {
        std::cerr << "Error loading player sprites." << std::endl;
        return 1;
    }
    // for (auto &color : player_sprite_data) {
    //     std::cout << glm::to_string(color) << "\n";
    // }
    std::cout << "Player sprites loaded successfully!" << std::endl;

    if (!write_player_sprites_file(&player_sprite_data)) {
        std::cerr << "Error writing player sprite data to .sprinfo file." << std::endl;
        return 1;
    }
    std::cout << "Player sprites successfully compiled to .sprinfo file!" << std::endl;
    return 0;
}
