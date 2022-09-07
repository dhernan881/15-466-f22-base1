#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#include "load_save_png.hpp"
#include "data_path.hpp"

bool load_palettes(std::vector<glm::u8vec4> *data) {
    glm::uvec2 palette_size(0, 0);

    std::string path = data_path("assets/palettes.png");

    // NOT sure if this is the correct OriginLocation.
    load_png(path, &palette_size, data, UpperLeftOrigin);

    // There should be 8 rows of 4 colors
    if (palette_size.x != 4 || palette_size.y != 8) {
        std::cerr << "Incorrect dimensions for palettes.png. Expected: 4x8. Got: "
            << palette_size.x << "x" << palette_size.y << std::endl;
        return false;
    }
    return true;
}

bool write_palette_file(std::vector<glm::u8vec4> *data) {
    std::string filename = data_path("gen/palettes.sprinfo");

    std::ofstream PaletteFile(filename, std::ios::out | std::ios::binary);

    for (auto &it : *data) {
        char data_to_write[5];
        data_to_write[0] = it.x;
        data_to_write[1] = it.y;
        data_to_write[2] = it.z;
        data_to_write[3] = it.w;
        data_to_write[4] = '\n';
        PaletteFile.write(data_to_write, sizeof(data_to_write));
    }

    PaletteFile.close();
    return true;
}

bool load_player_sprites(std::vector<glm::u8vec4> *data) {
    glm::uvec2 img_size(0, 0);

    std::string path = data_path("assets/spritesheets/player.png");

    // Same UNCERTAINTY about OriginLocation.
    load_png(path, &img_size, data, UpperLeftOrigin);

    // The sprites should be 16 8x8 sprites stacked on top of each other.
    if (img_size.x != 8 || img_size.y != (16 * 8)) {
        std::cerr << "Incorrect dimensions for palettes.png. Expected: 8x128. Got: "
            << img_size.x << "x" << img_size.y << std::endl;
        return false;
    }

    return true;
}
