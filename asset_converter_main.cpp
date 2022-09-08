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
    std::cout << "Water sprites successfully compiled to .sprinfo file!" << std::endl;

    // Load the bullet sprites.
    std::vector<glm::u8vec4> bullet_sprite_data;
    if (!load_sprites(&bullet_sprite_data, data_path("assets/spritesheets/bullet.png"),
            8, 8 * 8)) {
        std::cerr << "Error loading bullet sprites." << std::endl;
        return 1;
    }
    std::cout << "Bullet sprites loaded successfully!" << std::endl;
    // Write the bullet sprites to a .sprinfo file.
    if (!write_sprites_file(&bullet_sprite_data, data_path("gen/bullet.sprinfo"))) {
        std::cerr << "Error writing bullet sprite data to .sprinfo file." << std::endl;
        return 1;
    }
    std::cout << "Bullet sprites successfully compiled to .sprinfo file!" << std::endl;

    // Load the text sprites.
    std::vector<glm::u8vec4> text_sprite_data;
    if (!load_sprites(&text_sprite_data, data_path("assets/spritesheets/text.png"),
            8, 8 * 8)) {
        std::cerr << "Error loading text sprites." << std::endl;
        return 1;
    }
    std::cout << "Text sprites loaded successfully!" << std::endl;
    // Write the text sprites to a .sprinfo file.
    if (!write_sprites_file(&text_sprite_data, data_path("gen/text.sprinfo"))) {
        std::cerr << "Error writing text sprite data to .sprinfo file." << std::endl;
        return 1;
    }
    std::cout << "Text sprites successfully compiled to .sprinfo file!" << std::endl;
    return 0;
}
