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

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, w, a, s, d;

	//some weird background animation:
	float background_fade = 0.0f;

	std::array< glm::vec2, 16 > movement_dirs = {
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

	//player position:
	// glm::vec2 player_at = glm::vec2(0.0f);
	struct Player {
		void Rotate(int dir);
		// Will move input handling to player functions later to remove
		// duplicate code.

		glm::vec2 pos = glm::vec2(0.0f);
		float spd;
		glm::vec2 dir = glm::vec2(0.0f);
		uint8_t rotation_state = 0;
		float rotation_cooldown = 0;

		static constexpr float RotationDelay = 0.125f;
		static constexpr float PlayerSpeed = 30.0f;
		static constexpr float Deceleration = 0.975f;
		static constexpr float BrakeDecel = 0.9f;
	} player1, player2; // TODO: add default values for player1 and player2
	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
