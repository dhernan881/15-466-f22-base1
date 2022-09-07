#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iterator>
#include <random>
#include <iostream>
#include <fstream>

#include "Load.hpp"
#include "data_path.hpp"

Load< std::array< PPU466::Palette, 8 > > GamePalettes(LoadTagDefault, [](){
    std::string palette_path = data_path("gen/palettes.sprinfo");
    std::ifstream PaletteFile(palette_path, std::ios::in | std::ios::binary);

    auto ret = new std::array< PPU466::Palette, 8 >;

    char color_buf[5];
    uint i = 0;
    while (PaletteFile.getline(color_buf, 5)) {
        uint row = i / 4; // 4 colors per palette
        uint col = i % 4;
        glm::u8vec4 cur_color (color_buf[0], color_buf[1], color_buf[2], 
            color_buf[3]);
        (*ret)[row][col] = cur_color;
        ++i;
    }
    if (i != 32) {
        throw std::runtime_error("Invalid number of colors read in palette.");
    }

    PaletteFile.close();
    return ret;
});

Load< std::array< std::array< uint8_t , 8 >, 8 * 16 > > PlayerSpriteInfo(LoadTagDefault, []() {
	std::string sprite_path = data_path("gen/player.sprinfo");
	std::ifstream SpriteFile(sprite_path, std::ios::in | std::ios::binary);

	auto ret = new std::array< std::array< uint8_t , 8 >, 8 * 16 >;

	char color_buf[9];
	uint i = 0;
	while (SpriteFile.getline(color_buf, 9)) {
		std::copy(std::begin(color_buf), std::begin(color_buf) + 8, (*ret)[i].begin());
		++i;
	}
	if (i != 8 * 16) {
		throw std::runtime_error("Invalid number of colors read in player spritesheet.");
	}

	SpriteFile.close();
	return ret;
});

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	// { //use tiles 0-16 as some weird dot pattern thing:
	// 	std::array< uint8_t, 8*8 > distance;
	// 	for (uint32_t y = 0; y < 8; ++y) {
	// 		for (uint32_t x = 0; x < 8; ++x) {
	// 			float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
	// 			d /= glm::length(glm::vec2(4.0f, 4.0f));
	// 			distance[x+8*y] = uint8_t(std::max(0,std::min(255,int32_t( 255.0f * d ))));
	// 		}
	// 	}
	// 	for (uint32_t index = 0; index < 16; ++index) {
	// 		PPU466::Tile tile;
	// 		uint8_t t = uint8_t((255 * index) / 16);
	// 		for (uint32_t y = 0; y < 8; ++y) {
	// 			uint8_t bit0 = 0;
	// 			uint8_t bit1 = 0;
	// 			for (uint32_t x = 0; x < 8; ++x) {
	// 				uint8_t d = distance[x+8*y];
	// 				if (d > t) {
	// 					bit0 |= (1 << x);
	// 				} else {
	// 					bit1 |= (1 << x);
	// 				}
	// 			}
	// 			tile.bit0[y] = bit0;
	// 			tile.bit1[y] = bit1;
	// 		}
	// 		ppu.tile_table[index] = tile;
	// 	}
	// }

	// Load player sprites into the tile table
	for (uint y = 0; y < 8 * 16; y++) {
		ppu.tile_table[y / 8].bit0[7 - (y % 8)] = 0;
		ppu.tile_table[y / 8].bit1[7 - (y % 8)] = 0;
		for (uint x = 0; x < 8; x++) {
			uint8_t cur_pixel_color = (*PlayerSpriteInfo)[y][x];
			// isolate the 0th and 1th bit
			uint8_t pixel_b0 = cur_pixel_color & 0b01;
			uint8_t pixel_b1 = (cur_pixel_color & 0b10) >> 1;

			uint8_t cur_bit0 = ppu.tile_table[y / 8].bit0[7 - (y % 8)];
			cur_bit0 = cur_bit0 | (pixel_b0 << x);
			ppu.tile_table[y / 8].bit0[7 - (y % 8)] = cur_bit0;
			uint8_t cur_bit1 = ppu.tile_table[y / 8].bit1[7 - (y % 8)];
			cur_bit1 = cur_bit1 | (pixel_b1 << x);
			ppu.tile_table[y / 8].bit1[7 - (y % 8)] = cur_bit1;
		}
	}

	//use sprite 32 as a "player":
	// ppu.tile_table[32].bit0 = {
	// 	0b01111110,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b01111110,
	// };
	// ppu.tile_table[32].bit1 = {
	// 	0b00100100,
	// 	0b00111100,
	// 	0b00111100,
	// 	0b00011000,
	// 	0b01100110,
	// 	0b01100110,
	// 	0b00000000,
	// 	0b00000000,
	// };

	ppu.palette_table = *GamePalettes;

	//makes the outside of tiles 0-16 solid:
	// ppu.palette_table[0] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	// //makes the center of tiles 0-16 solid:
	// ppu.palette_table[1] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	// //used for the player:
	// ppu.palette_table[7] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0xff, 0xff, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	// //used for the misc other sprites:
	// ppu.palette_table[6] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x88, 0x88, 0xff, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// };
	// for (uint i = 0; i < 8; i++) {
	// 	std::cout << "color " << i << ": ";
	// 	std::cout << glm::to_string(ppu.palette_table[i][0]);
	// 	std::cout << glm::to_string(ppu.palette_table[i][1]);
	// 	std::cout << glm::to_string(ppu.palette_table[i][2]);
	// 	std::cout << glm::to_string(ppu.palette_table[i][3]);
	// 	std::cout << std::endl;
	// }
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
			ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
		}
	}

	//background scroll:
	ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprite:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 0;
	ppu.sprites[0].attributes = 0b00000001;

	//some other misc sprites:
	for (uint32_t i = 1; i < 63; ++i) {
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int8_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f + 0.01f * player_at.x) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int8_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].index = 32;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
