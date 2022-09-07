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

	player1.pos = glm::vec2(0.0f);
	player2.pos = glm::vec2(256.0 - 8.0f, 240.0 - 8.0f);
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
		} else if (evt.key.keysym.sym == SDLK_w) {
			w.downs += 1;
			w.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			a.downs += 1;
			a.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			s.downs += 1;
			s.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			d.downs += 1;
			d.pressed = true;
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
		} else if (evt.key.keysym.sym == SDLK_w) {
			w.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			a.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			s.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			d.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::Player::Rotate(int dir) {
	if (rotation_cooldown > 0.0f)
		return;
	rotation_cooldown = Player::RotationDelay;
	switch (dir) {
		case 1:
			rotation_state = (rotation_state + 1) % 16;
			break;
		case -1:
			// weird bug where mod not working properly??
			rotation_state = (rotation_state + 15) % 16;
			break;
		default:
			std::cerr << "Invalid rotation value: " << dir << "\n";
	}
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	// if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	// if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	// if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	// if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	// HANDLE player1 MOVEMENT
	if (a.pressed && !d.pressed) {
		player1.Rotate(-1);
	}
	if (!a.pressed && d.pressed) {
		player1.Rotate(1);
	}
	if (w.pressed) {
		glm::vec2 tmp_dir = movement_dirs[player1.rotation_state];
		tmp_dir += player1.dir;
		player1.dir = glm::normalize(tmp_dir);
		player1.spd = Player::PlayerSpeed;
		// add acceleration later?
	} else if (s.pressed) {
		player1.spd *= Player::BrakeDecel;
	} else {
		player1.spd *= Player::Deceleration;
	}
	player1.pos.x += player1.spd * elapsed * player1.dir.x;
	player1.pos.y += player1.spd * elapsed * player1.dir.y;
	// Decrement rotation cooldown (and decelerate?)
	player1.rotation_cooldown -= elapsed;

	// HANDLE player2 MOVEMENT
	// For now, copy-pasted from player1. Will move to be a member of Player later.
	if (left.pressed && !right.pressed) {
		player2.Rotate(-1);
	}
	if (!left.pressed && right.pressed) {
		player2.Rotate(1);
	}
	if (up.pressed) {
		glm::vec2 tmp_dir = movement_dirs[player2.rotation_state];
		tmp_dir += player2.dir;
		player2.dir = glm::normalize(tmp_dir);
		player2.spd = Player::PlayerSpeed;
		// add acceleration later?
	} else if (down.pressed) {
		player2.spd *= Player::BrakeDecel;
	} else {
		player2.spd *= Player::Deceleration;
	}
	player2.pos.x += player2.spd * elapsed * player2.dir.x;
	player2.pos.y += player2.spd * elapsed * player2.dir.y;
	// Decrement rotation cooldown (and decelerate?)
	player2.rotation_cooldown -= elapsed;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	w.downs = 0;
	a.downs = 0;
	s.downs = 0;
	d.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	// ppu.background_color = glm::u8vec4(
	// 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
	// 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
	// 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
	// 	0xff
	// );
	ppu.background_color = glm::u8vec4 (0, 0xff, 0xff, 0xff);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
			ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
		}
	}

	//background scroll:
	// ppu.background_position.x = int32_t(-0.5f * player1.pos.x);
	// ppu.background_position.y = int32_t(-0.5f * player1.pos.y);

	//player1 sprite:
	ppu.sprites[0].x = int8_t(player1.pos.x);
	ppu.sprites[0].y = int8_t(player1.pos.y);
	ppu.sprites[0].index = player1.rotation_state;
	ppu.sprites[0].attributes = 0b00000001;

	//player2 sprite:
	ppu.sprites[1].x = int8_t(player2.pos.x);
	ppu.sprites[1].y = int8_t(player2.pos.y);
	ppu.sprites[1].index = player2.rotation_state;
	ppu.sprites[1].attributes = 0b00000010;

	//some other misc sprites:
	// for (uint32_t i = 2; i < 63; ++i) {
	// 	float amt = (i + 2.0f * background_fade) / 62.0f;
	// 	ppu.sprites[i].x = int8_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f + 0.01f * player1.pos.x) * 0.4f * PPU466::ScreenWidth);
	// 	ppu.sprites[i].y = int8_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player1.pos.y) * 0.4f * PPU466::ScreenWidth);
	// 	ppu.sprites[i].index = 32;
	// 	ppu.sprites[i].attributes = 6;
	// 	if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	// }

	//--- actually draw ---
	ppu.draw(drawable_size);
}
