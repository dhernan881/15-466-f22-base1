#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <cmath>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	void draw_player_info();
	void draw_winner();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} w, a, s, d, lshift, i, j, k, l, period;

	// Background information:
	static constexpr uint num_water_details = 100;
	static constexpr float bg_scroll_speed = 10.0f;
	double total_time = 0.0;

	static constexpr std::array< glm::vec2, 16 > movement_dirs = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.382683f, 0.923879f),
		glm::vec2(0.707106f, 0.707106f),
		glm::vec2(0.923879f, 0.382683f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.923879f, -0.382683f),
		glm::vec2(0.707106f, -0.707106f),
		glm::vec2(0.382683f, -0.923879f),
		glm::vec2(0.0f, -1.0f),
		glm::vec2(-0.382683f, -0.923879f),
		glm::vec2(-0.707106f, -0.707106f),
		glm::vec2(-0.923879f, -0.382683f),
		glm::vec2(-1.0f, 0.0f),
		glm::vec2(-0.923879f, 0.382683f),
		glm::vec2(-0.707106f, 0.707106f),
		glm::vec2(-0.382683f, 0.923879f)
	};

	struct Bullet;

	//player position:
	struct Player {
		// Properties
		static constexpr float RotationDelay = 0.125f;
		static constexpr float ShootDelay = 0.125f;
		static constexpr float PlayerSpeed = 30.0f;
		static constexpr float Deceleration = 0.975f;
		static constexpr float BrakeDecel = 0.9f;
		static constexpr uint MaxBullets = 3;
		static constexpr int MaxHealth = 3;

		void TryRotate(int dir);
		void TryShoot(int player_num, int target_num);
		void DecrementHealth(uint amt, uint player_num);
		// Will move input handling to player functions later to remove
		// duplicate code.

		glm::vec2 pos = glm::vec2(0.0f);
		float spd = 0.0f;
		int health = Player::MaxHealth;
		glm::vec2 dir = glm::vec2(0.0f);
		uint8_t rotation_state = 0;
		float rotation_cooldown = 0.0f;
		float shoot_cooldown = 0.0f;

		PlayMode::Bullet *active_bullets[PlayMode::Player::MaxBullets] = 
			{nullptr, nullptr, nullptr};
		std::deque< uint > bullets_to_delete;
	} player1, player2; // TODO: add default values for player1 and player2

	struct Bullet {
		// Properties
		static constexpr float Lifespan = 3.0f;
		static constexpr float BulletSpeed = 60.0f;
		static constexpr float AnimDelay = 0.25f;

		Bullet(uint owner_num_, uint target_num_, uint list_idx) {
			// From https://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
			if (owner_num_ == 1) {
				owner = &((std::dynamic_pointer_cast<PlayMode>(Mode::current))->player1);
				owner_num = 1;
				target = &((std::dynamic_pointer_cast<PlayMode>(Mode::current))->player2);
				target_num = 2;
			} else {
				owner = &((std::dynamic_pointer_cast<PlayMode>(Mode::current))->player2);
				owner_num = 2;
				target = &((std::dynamic_pointer_cast<PlayMode>(Mode::current))->player1);
				target_num = 1;
			}
			pos = owner->pos;
			dir = PlayMode::movement_dirs[owner->rotation_state];
			owner_list_idx = list_idx;
		};
		void Update(float elapsed);
		void Die();

		glm::vec2 pos = glm::vec2(0.0f);
		glm::vec2 dir = glm::vec2(0.0f);
		PlayMode::Player *owner = nullptr;
		uint owner_num = 0;
		PlayMode::Player *target = nullptr;
		uint target_num = 0;
		uint owner_list_idx = 0;
		float life = Bullet::Lifespan;
		int dmg = 1;

		uint8_t anim_state = 0;
		float anim_cooldown = Bullet::AnimDelay;
	};

	uint winner = 0;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
