#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#include "load_save_png.hpp"
#include "data_path.hpp"

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

bool load_sprites(std::vector<glm::u8vec4> *data, std::string path,
    uint expected_x, uint expected_y) {
    glm::uvec2 img_size(0, 0);

    // Same UNCERTAINTY about OriginLocation.
    load_png(path, &img_size, data, UpperLeftOrigin);

    // The sprites should be 8x8 sprites stacked on top of each other.
    if (img_size.x != expected_x || img_size.y != expected_y) {
        std::cerr << "Incorrect dimensions for" << path << ". Expected: " <<
            expected_x << "x" << expected_y << ". Got: " << img_size.x << "x" <<
            img_size.y << std::endl;
        return false;
    }

    return true;
}

bool write_sprites_file(std::vector<glm::u8vec4> *data, std::string path) {
    std::ofstream SpriteFile(path, std::ios::out | std::ios::binary);

    uint i = 0;
    for (auto &it : *data) {
        char color = it.x / 85;
        SpriteFile.write(&color, 1);
        if (i % 8 == 7) {
            color = '\n';
            SpriteFile.write(&color, 1);
        }
        ++i;
    }

    SpriteFile.close();
    return true;
}
