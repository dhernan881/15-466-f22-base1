#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iterator>
#include <random>
#include <iostream>
#include <fstream>

#include "Load.hpp"
#include "data_path.hpp"

double background_scroll_fn(double t) {
	return 6.0 * std::sin(0.1 * t) + 0.3 * t;
}

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

Load< std::array< std::array< uint8_t , 8 >, 8 * 8 > > WaterSpriteInfo(LoadTagDefault, [] () {
	std::string sprite_path = data_path("gen/water.sprinfo");
	std::ifstream SpriteFile(sprite_path, std::ios::in | std::ios::binary);

	auto ret = new std::array< std::array< uint8_t , 8 >, 8 * 8 >;

	char color_buf[9];
	uint i = 0;
	while (SpriteFile.getline(color_buf, 9)) {
		std::copy(std::begin(color_buf), std::begin(color_buf) + 8, (*ret)[i].begin());
		++i;
	}
	if (i != 8 * 8) {
		throw std::runtime_error("Invalid number of colors read in water spritesheet.");
	}

	SpriteFile.close();
	return ret;
});

Load< std::array< std::array< uint8_t , 8 >, 8 * 8 > > BulletSpriteInfo(LoadTagDefault, [] () {
	std::string sprite_path = data_path("gen/bullet.sprinfo");
	std::ifstream SpriteFile(sprite_path, std::ios::in | std::ios::binary);

	auto ret = new std::array< std::array< uint8_t , 8 >, 8 * 8 >;

	char color_buf[9];
	uint i = 0;
	while (SpriteFile.getline(color_buf, 9)) {
		std::copy(std::begin(color_buf), std::begin(color_buf) + 8, (*ret)[i].begin());
		++i;
	}
	if (i != 8 * 8) {
		throw std::runtime_error("Invalid number of colors read in bullet spritesheet.");
	}

	SpriteFile.close();
	return ret;
});

Load< std::array< std::array< uint8_t , 8 >, 8 * 8 > > TextSpriteInfo(LoadTagDefault, [] () {
	std::string sprite_path = data_path("gen/text.sprinfo");
	std::ifstream SpriteFile(sprite_path, std::ios::in | std::ios::binary);

	auto ret = new std::array< std::array< uint8_t , 8 >, 8 * 8 >;

	char color_buf[9];
	uint i = 0;
	while (SpriteFile.getline(color_buf, 9)) {
		std::copy(std::begin(color_buf), std::begin(color_buf) + 8, (*ret)[i].begin());
		++i;
	}
	if (i != 8 * 8) {
		throw std::runtime_error("Invalid number of colors read in text spritesheet.");
	}

	SpriteFile.close();
	return ret;
});

PlayMode::PlayMode() {
	// Load the palettes into the ppu's palette_table.
	ppu.palette_table = *GamePalettes;

	player1.pos = glm::vec2(0.0f);
	player1.rotation_state = 2;
	player2.pos = glm::vec2(256.0 - 8.0f, 240.0 - 8.0f);
	player2.rotation_state = 10;

	// Load player sprites into the tile table.
	// May make a separate function for this later.
	// Tiles 0-15 are reserved for the player animations.
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
	// Tiles 16-63 are reserved for if I decide to expand the player to 16x16.

	// Load the water sprites into the tile table.
	// Same as for the player but with an offset.
	// Tiles 64-71 are reserved for water details.
	{
		const uint offset = 64;
		for (uint y = 0; y < 8 * 8; y++) {
			ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)] = 0;
			ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)] = 0;
			for (uint x = 0; x < 8; x++) {
				uint8_t cur_pixel_color = (*WaterSpriteInfo)[y][x];
				// isolate the 0th and 1th bit
				uint8_t pixel_b0 = cur_pixel_color & 0b01;
				uint8_t pixel_b1 = (cur_pixel_color & 0b10) >> 1;

				uint8_t cur_bit0 = ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)];
				cur_bit0 = cur_bit0 | (pixel_b0 << x);
				ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)] = cur_bit0;
				uint8_t cur_bit1 = ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)];
				cur_bit1 = cur_bit1 | (pixel_b1 << x);
				ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)] = cur_bit1;
			}
		}
	}
	// Tiles 72-79 are reserved for additional water details/animations.

	// Load the bullet sprites into the tile table.
	// Same as for the water.
	// Tiles 80-87 are reserved for bullet animations.
	{
		const uint offset = 80;
		for (uint y = 0; y < 8 * 8; y++) {
			ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)] = 0;
			ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)] = 0;
			for (uint x = 0; x < 8; x++) {
				uint8_t cur_pixel_color = (*BulletSpriteInfo)[y][x];
				// isolate the 0th and 1th bit
				uint8_t pixel_b0 = cur_pixel_color & 0b01;
				uint8_t pixel_b1 = (cur_pixel_color & 0b10) >> 1;

				uint8_t cur_bit0 = ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)];
				cur_bit0 = cur_bit0 | (pixel_b0 << x);
				ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)] = cur_bit0;
				uint8_t cur_bit1 = ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)];
				cur_bit1 = cur_bit1 | (pixel_b1 << x);
				ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)] = cur_bit1;
			}
		}
	}

	// Load the text sprites into the tile table.
	// Same as for the bullets.
	// Tiles 88-95 are reserved for text.
	{
		const uint offset = 88;
		for (uint y = 0; y < 8 * 8; y++) {
			ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)] = 0;
			ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)] = 0;
			for (uint x = 0; x < 8; x++) {
				uint8_t cur_pixel_color = (*TextSpriteInfo)[y][x];
				// isolate the 0th and 1th bit
				uint8_t pixel_b0 = cur_pixel_color & 0b01;
				uint8_t pixel_b1 = (cur_pixel_color & 0b10) >> 1;

				uint8_t cur_bit0 = ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)];
				cur_bit0 = cur_bit0 | (pixel_b0 << x);
				ppu.tile_table[(y / 8) + offset].bit0[7 - (y % 8)] = cur_bit0;
				uint8_t cur_bit1 = ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)];
				cur_bit1 = cur_bit1 | (pixel_b1 << x);
				ppu.tile_table[(y / 8) + offset].bit1[7 - (y % 8)] = cur_bit1;
			}
		}
	}
	// Tiles 96-103 are reserved for additional text character sprites.

	// Tile 255 is fully-transparent.
	ppu.tile_table[255].bit0 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	ppu.tile_table[255].bit1 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	// Initialize background to fully-transparent.
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			// Tile 255, palette 0.
			ppu.background[x+PPU466::BackgroundWidth*y] = 0xFF;
		}
	}
	// Place random details in the background.
	{
		const uint16_t bg_base = 0b11 << 8; // Use the bg palette.
		// From https://cplusplus.com/reference/random/uniform_int_distribution/
		std::default_random_engine generator;
		std::uniform_int_distribution<uint> x_dist(0, ppu.BackgroundWidth - 1);
		std::uniform_int_distribution<uint> y_dist(0, ppu.BackgroundHeight - 1);
		std::uniform_int_distribution<uint> water_dist(64, 64 + 8 - 1);
		for (uint i = 0; i < num_water_details; i++) {
			uint x = x_dist(generator);
			uint y = y_dist(generator);
			uint tile = water_dist(generator);
			ppu.background[x + PPU466::BackgroundWidth * y] = bg_base | tile;
		}
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_w) {
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
		} else if (evt.key.keysym.sym == SDLK_LSHIFT) {
			lshift.downs += 1;
			lshift.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			i.downs += 1;
			i.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_j) {
			j.downs += 1;
			j.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_k) {
			k.downs += 1;
			k.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_l) {
			l.downs += 1;
			l.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_PERIOD) {
			period.downs += 1;
			period.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_w) {
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
		} else if (evt.key.keysym.sym == SDLK_LSHIFT) {
			lshift.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			i.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_j) {
			j.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_k) {
			k.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_l) {
			l.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_PERIOD) {
			period.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::Player::TryRotate(int dir) {
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

void PlayMode::Player::TryShoot(int player_num, int target_num) {
	if (shoot_cooldown > 0.0f)
		return;
	for (uint i = 0; i < PlayMode::Player::MaxBullets; i++) {
		if (active_bullets[i] != nullptr)
			continue;
		// Spawn a bullet and add it to the active bullets set at this index
		PlayMode::Bullet *bullet = new PlayMode::Bullet(player_num, target_num, i);
		active_bullets[i] = bullet;
		shoot_cooldown = Player::ShootDelay;
		break;
	}
}

// TODO: remove the player arg by having a better way to identify
// player1 and player2.
void PlayMode::Player::DecrementHealth(uint amt, uint player_num) {
	health -= amt;
	if (health <= 0 && (std::dynamic_pointer_cast<PlayMode>(Mode::current))->winner == 0) {
		// Alert game that winner.
		if (player_num == 1)
			(std::dynamic_pointer_cast<PlayMode>(Mode::current))->winner = 2;
		else
			(std::dynamic_pointer_cast<PlayMode>(Mode::current))->winner = 1;
	}
}

void PlayMode::Bullet::Update(float elapsed) {
	PPU466 *ppu = &((std::dynamic_pointer_cast<PlayMode>(Mode::current))->ppu);
	pos += dir * Bullet::BulletSpeed * elapsed;
	if (pos.x > ppu->ScreenWidth)
		pos.x -= ppu->ScreenWidth;
	if (pos.x < 0)
		pos.x += ppu->ScreenWidth;
	if (pos.y > ppu->ScreenHeight)
		pos.y -= ppu->ScreenHeight;
	if (pos.y < 0)
		pos.y += ppu->ScreenHeight;
	life -= elapsed;
	anim_cooldown -= elapsed;
	// Check for collision with target 
	if (glm::distance(pos, target->pos) < 8.0f) { 
		target->DecrementHealth(dmg, target_num);
		Die();
		return;
	}
	if (life <= 0.0f) {
		Die();
		return;
	}
	if (anim_cooldown <= 0.0f) {
		anim_state = (anim_state + 1) % 8;
		anim_cooldown = Bullet::AnimDelay;
	}
}

void PlayMode::Bullet::Die() {
	owner->bullets_to_delete.push_back(owner_list_idx);
}

void PlayMode::update(float elapsed) {
	// HANDLE player1 MOVEMENT
	{
		if (a.pressed && !d.pressed) {
			player1.TryRotate(-1);
		}
		if (!a.pressed && d.pressed) {
			player1.TryRotate(1);
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
		player1.rotation_cooldown -= elapsed;
		if (player1.pos.x > ppu.ScreenWidth)
			player1.pos.x -= ppu.ScreenWidth;
		if (player1.pos.x < 0)
			player1.pos.x += ppu.ScreenWidth;
		if (player1.pos.y > ppu.ScreenHeight)
			player1.pos.y -= ppu.ScreenHeight;
		if (player1.pos.y < 0)
			player1.pos.y += ppu.ScreenHeight;
	}

	// HANDLE player2 MOVEMENT
	{
		// For now, copy-pasted from player1. Will move to be a member of Player later.
		if (j.pressed && !l.pressed) {
			player2.TryRotate(-1);
		}
		if (!j.pressed && l.pressed) {
			player2.TryRotate(1);
		}
		if (i.pressed) {
			glm::vec2 tmp_dir = movement_dirs[player2.rotation_state];
			tmp_dir += player2.dir;
			player2.dir = glm::normalize(tmp_dir);
			player2.spd = Player::PlayerSpeed;
			// add acceleration later?
		} else if (k.pressed) {
			player2.spd *= Player::BrakeDecel;
		} else {
			player2.spd *= Player::Deceleration;
		}
		player2.pos.x += player2.spd * elapsed * player2.dir.x;
		player2.pos.y += player2.spd * elapsed * player2.dir.y;
		player2.rotation_cooldown -= elapsed;
		if (player2.pos.x > ppu.ScreenWidth)
			player2.pos.x -= ppu.ScreenWidth;
		if (player2.pos.x < 0)
			player2.pos.x += ppu.ScreenWidth;
		if (player2.pos.y > ppu.ScreenHeight)
			player2.pos.y -= ppu.ScreenHeight;
		if (player2.pos.y < 0)
			player2.pos.y += ppu.ScreenHeight;
	}

	// HANDLE player1 BULLETS
	{
		// If player1 pushed the shoot button, try to spawn a bullet.
		if (lshift.pressed) {
			player1.TryShoot(1, 2);
		}
		// Update bullets.
		for (uint i = 0; i < Player::MaxBullets; i++) {
				if (player1.active_bullets[i] == nullptr)
					continue;
				player1.active_bullets[i]->Update(elapsed);
			}
		// Delete bullets that should be dead.
		// From https://en.cppreference.com/w/cpp/container/deque/pop_front
		for (; !player1.bullets_to_delete.empty(); player1.bullets_to_delete.pop_front()) {
			uint bullet_idx = player1.bullets_to_delete.front();
			delete player1.active_bullets[bullet_idx];
			player1.active_bullets[bullet_idx] = nullptr;
		}
		player1.shoot_cooldown -= elapsed;
	}

	// HANDLE player2 BULLETS
	{
		// If player2 pushed the shoot button, try to spawn a bullet.
		if (period.pressed) {
			player2.TryShoot(2, 1);
		}
		// Update bullets.
		for (uint i = 0; i < Player::MaxBullets; i++) {
				if (player2.active_bullets[i] == nullptr)
					continue;
				player2.active_bullets[i]->Update(elapsed);
			}
		// Delete bullets that should be dead.
		// From https://en.cppreference.com/w/cpp/container/deque/pop_front
		for (; !player2.bullets_to_delete.empty(); player2.bullets_to_delete.pop_front()) {
			uint bullet_idx = player2.bullets_to_delete.front();
			delete player2.active_bullets[bullet_idx];
			player2.active_bullets[bullet_idx] = nullptr;
		}
		player2.shoot_cooldown -= elapsed;
	}

	//reset button press counters:
	w.downs = 0;
	a.downs = 0;
	s.downs = 0;
	d.downs = 0;
	lshift.downs = 0;
	i.downs = 0;
	j.downs = 0;
	k.downs = 0;
	l.downs = 0;
	period.downs = 0;

	// Update global game timer
	total_time += elapsed;
}

void PlayMode::draw_player_info() {
	uint tile_offset = 88;
	uint sprite_offset = 2 + 2 * Player::MaxBullets; // should be 8
	
	// player1 'P'
	ppu.sprites[sprite_offset].x = 5 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 3 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 3;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// player1 '1'
	ppu.sprites[sprite_offset].x = 6 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 3 * 8;
	ppu.sprites[sprite_offset].index = tile_offset;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// player1 health
	ppu.sprites[sprite_offset].x = 8 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 3 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + std::max(player1.health - 1, 0);
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// player2 'P'
	ppu.sprites[sprite_offset].x = ppu.ScreenWidth - 8 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 3 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 3;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// player2 '2'
	ppu.sprites[sprite_offset].x = ppu.ScreenWidth - 7 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 3 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 1;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// player2 health
	ppu.sprites[sprite_offset].x = ppu.ScreenWidth - 5 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 3 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + std::max(player2.health - 1, 0);
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
}

void PlayMode::draw_winner() {
	uint tile_offset = 88;
	uint sprite_offset = 2 + 2 * Player::MaxBullets + 6; // should be 14

	// 'P'
	ppu.sprites[sprite_offset].x = (ppu.ScreenWidth / 2) - 4 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 10 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 3;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// winner player number
	ppu.sprites[sprite_offset].x = (ppu.ScreenWidth / 2) - 3 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 10 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + (winner - 1);
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// 'W'
	ppu.sprites[sprite_offset].x = (ppu.ScreenWidth / 2) - 1 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 10 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 4;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// 'I'
	ppu.sprites[sprite_offset].x = (ppu.ScreenWidth / 2);
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 10 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 5;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// 'N'
	ppu.sprites[sprite_offset].x = (ppu.ScreenWidth / 2) + 1 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 10 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 6;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
	// '!'
	ppu.sprites[sprite_offset].x = (ppu.ScreenWidth / 2) + 2 * 8;
	ppu.sprites[sprite_offset].y = ppu.ScreenHeight - 10 * 8;
	ppu.sprites[sprite_offset].index = tile_offset + 7;
	ppu.sprites[sprite_offset].attributes = 0;
	++sprite_offset;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	ppu.background_color = glm::u8vec4 (0, 0x50, 0xff, 0xff);

	// background:
	ppu.background_position.x = total_time * bg_scroll_speed;
	ppu.background_position.y = background_scroll_fn(total_time * bg_scroll_speed);

	// player1 sprite:
	ppu.sprites[0].x = int8_t(player1.pos.x);
	ppu.sprites[0].y = int8_t(player1.pos.y);
	ppu.sprites[0].index = player1.rotation_state;
	ppu.sprites[0].attributes = 0b00000001;

	// player2 sprite:
	ppu.sprites[1].x = int8_t(player2.pos.x);
	ppu.sprites[1].y = int8_t(player2.pos.y);
	ppu.sprites[1].index = player2.rotation_state;
	ppu.sprites[1].attributes = 0b00000010;
	
	// player1 bullet sprites:
	{
		uint tile_offset = 80;
		uint sprite_offset = 2;
		for (uint i = 0; i < Player::MaxBullets; i++) {
			if (player1.active_bullets[i] == nullptr) {
				ppu.sprites[i + sprite_offset].y = 240;
				continue;
			}
			ppu.sprites[i + sprite_offset].x = 
				int8_t(player1.active_bullets[i]->pos.x);
			ppu.sprites[i + sprite_offset].y = 
				int8_t(player1.active_bullets[i]->pos.y);
			ppu.sprites[i + sprite_offset].index =
				tile_offset + player1.active_bullets[i]->anim_state;
			ppu.sprites[i + sprite_offset].attributes = 0b00000001;
		}
	}

	// player2 bullet sprites:
	{
		uint tile_offset = 80;
		uint sprite_offset = 2 + Player::MaxBullets; // should be 5
		for (uint i = 0; i < Player::MaxBullets; i++) {
			if (player2.active_bullets[i] == nullptr) {
				ppu.sprites[i + sprite_offset].y = 240;
				continue;
			}
			ppu.sprites[i + sprite_offset].x = 
				int8_t(player2.active_bullets[i]->pos.x);
			ppu.sprites[i + sprite_offset].y = 
				int8_t(player2.active_bullets[i]->pos.y);
			ppu.sprites[i + sprite_offset].index =
				tile_offset + player2.active_bullets[i]->anim_state;
			ppu.sprites[i + sprite_offset].attributes = 0b00000010;
		}
	}

	// text sprites:
	draw_player_info(); // 6 sprites
	if (winner != 0) {
		draw_winner(); // 6 sprites
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
