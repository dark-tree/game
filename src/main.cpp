
#include <external.hpp>
#include <rendering.hpp>
#include <render/layout.hpp>

#include "const.hpp"
#include "game/sounds.hpp"
#include "game/level/level.hpp"

// docs
// https://emscripten.org/docs/api_reference/html5.h.html

// the function called by the javascript code
extern "C" void EXPORTED_NATIVE toggle_background_color() {

}

void checkViewport(float ratio, const std::function<void(int, int, int, int, glm::mat4& matrix)>& on_resize) {

	static int pw = 0;
	static int ph = 0;

	const auto [w, h] = getCanvasSize();

	if (w != pw || h != ph) {

		const float sx = 2.0f / w;
		const float sy = 2.0f / h;

		float rw = 0;
		float rh = 0;

		if (w > h * ratio) {
			rh = h;
			rw = h * ratio;
		} else {
			rh = w / ratio;
			rw = w;
		}

		// margin
		const float mx = (w - rw) / 2.0f;
		const float my = (h - rh) / 2.0f;

		// offset
		const float ox = sx * mx - 1;
		const float oy = sy * my - 1;

		// factor
		const float fx = sx * (rw / SW);
		const float fy = sy * (rh / SH);

		glm::mat4 matrix {
			fx * SW,   0,   0,   0,
			 0,  fy * SH,   0,   0,
			 0,   0,   1,   0,
			ox,  oy,   0,   1,
		};

		pw = w;
		ph = h;

		int rwi = rw;
		int rhi = rh;

		// resolution needs to be divisible by 2, not gonna lie, i don't know why
		if (rwi % 2 == 1) rwi ++;
		if (rhi % 2 == 1) rhi ++;

		on_resize(w, h, rwi, rhi, matrix);
		printf("Screen resized to %dx%d (with region: [%d, %d], offset: [%f, %f], factor: [%f, %f])\n", w, h, rwi, rhi, ox, oy, fx * SW, fy * SH);
	}

}

void loadBiomes(BiomeManager& biomes) {
	biomes.beginBiome() // start
		.setTerrain(0.0, 0.25)
		.addAlien(Alien::MINE, 1)
		//.addAlien(Alien::FIGHTER, 1)
		.addEvolution(Evolution::LOW, 1)
		.addEvolution(Evolution::MEDIUM, 1)
		.addEvolution(Evolution::HIGH, 1)
		.addEnemyPlacer(1, 0)
		.setPowerUpRarity(30)
		.setEndSegment(6);

	biomes.beginBiome() // sweepers only
		.setTerrain(0.0, 0.25)
		.addAlien(Alien::SWEEPER, 2)
		.addAlien(Alien::MINE, 1)
		.addEvolution(Evolution::LOW, 1)
		.addEvolution(Evolution::MEDIUM, 1)
		.addEnemyPlacer(1, 0)
		.setPowerUpRarity(30)
		.setEndSegment(20);

	biomes.beginBiome() // turret introduced
		.setTerrain(0.0, 0.27)
		.addAlien(Alien::SWEEPER, 3)
		.addAlien(Alien::TURRET, 1)
		.addAlien(Alien::MINE, 1)
		.addEvolution(Evolution::LOW, 1)
		.addEvolution(Evolution::MEDIUM, 1)
		.addEnemyPlacer(1, 0)
		.setEndSegment(60);

	biomes.beginBiome() // strong sweepers
		.setTerrain(0.1, 0.28)
		.addAlien(Alien::SWEEPER, 3)
		.addEvolution(Evolution::HIGH, 1)
		.addEnemyPlacer(1, 0)
		.addEnemyPlacer(1, 1)
		.setEndSegment(72);

	biomes.beginBiome() // a bit stronger sweeper-turret mix
		.setTerrain(0.0, 0.26)
		.addAlien(Alien::SWEEPER, 3)
		.addAlien(Alien::TURRET, 2)
		.addAlien(Alien::MINE, 1)
		.addAlien(Alien::VERTICAL, 2)
		.addEvolution(Evolution::LOW, 1)
		.addEvolution(Evolution::MEDIUM, 3)
		.addEnemyPlacer(1, 0)
		.setEndSegment(100);

	biomes.beginBiome() // now onto the HIGH evolution!
		.setTerrain(0.0, 0.26)
		.addAlien(Alien::SWEEPER, 1)
		.addAlien(Alien::TURRET, 1)
		.addEvolution(Evolution::LOW, 1)
		.addEvolution(Evolution::MEDIUM, 4)
		.addEvolution(Evolution::HIGH, 1)
		.addEnemyPlacer(1, 0)
		.addEnemyPlacer(1, 4)
		.setEndSegment(140);

	biomes.beginBiome() // i heard you like turrets?
		.setTerrain(0.0, 0.27)
		.addAlien(Alien::SWEEPER, 1)
		.addAlien(Alien::TURRET, 5)
		.addEvolution(Evolution::LOW, 2)
		.addEvolution(Evolution::MEDIUM, 2)
		.addEvolution(Evolution::HIGH, 1)
		.addEnemyPlacer(1, 0)
		.addEnemyPlacer(1, 4)
		.setEndSegment(180);

	biomes.beginBiome() // fun time
		.setTerrain(0.0, 0.26)
		.addAlien(Alien::SWEEPER, 1)
		.addAlien(Alien::TURRET, 1)
		.addEvolution(Evolution::MEDIUM, 3)
		.addEvolution(Evolution::HIGH, 1)
		.addEnemyPlacer(1, 0)
		.addEnemyPlacer(1, 5)
		.setEndSegment(-1);
}

int main() {

	auto begin_time = std::chrono::steady_clock::now();

	Vert2f vertices_quad[] = {
		{0, 0},
		{1, 0},
		{1, 1},
		{1, 1},
		{0, 1},
		{0, 0}
	};

    initVideoSystem();
	SoundSystem& system = SoundSystem::getInstance();
	Sounds::load();

	system.listener().gain(0.3f);

	BiomeManager biomes;
	loadBiomes(biomes);

	Level level {biomes};
	level.spawnInitial();

	Framebuffer pass_1;
	const Framebuffer& pass_2 = Framebuffer::main();

	Texture color_att;
	color_att.resize(SW, SH, GL_RGBA, GL_RGBA);

	RenderBuffer depth_att;
	depth_att.resize(SW, SH, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8);

	color_att.use();
	pass_1.attach(color_att, GL_COLOR_ATTACHMENT0);

	depth_att.use();
	pass_1.attach(depth_att, GL_DEPTH_STENCIL_ATTACHMENT);

	// Create and compile the shader program
	Shader level_shader {"assets/shader/level"};
	Shader degrade_shader {"assets/shader/degrade"};

	// Create buffer layout
	Layout geometry_layout;
	geometry_layout.attribute(level_shader.attribute("iPos"), 2, GL_FLOAT);
	geometry_layout.attribute(level_shader.attribute("iTex"), 2, GL_FLOAT);
	geometry_layout.attribute(level_shader.attribute("iCol"), 4, GL_UNSIGNED_BYTE, true);

	Layout screen_layout;
	screen_layout.attribute(degrade_shader.attribute("iPos"), 2, GL_FLOAT);

	VertexBuffer blit_buffer {screen_layout, GL_STATIC_DRAW};
	blit_buffer.upload((uint8_t*) vertices_quad, sizeof(vertices_quad));

	TileSet font8x8 {"assets/font8x8.png", 8};
	TileSet tileset {"assets/tileset.png", 16};

	VertexBuffer game_buffer {geometry_layout, GL_DYNAMIC_DRAW};
	BufferWriter<Vert4f4b> game_writer {game_buffer};

	VertexBuffer text_buffer {geometry_layout, GL_DYNAMIC_DRAW};
	BufferWriter<Vert4f4b> text_writer {text_buffer};

	setBlend(true);

	printf("System ready!\n");

	int __w, __h;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	setMainLoop([&] {

		level.tick();

		// takes care of the screen ratio, calls the callback when the screen resizes
		checkViewport(ASPECT_RATIO, [&] (int w, int h, int rw, int rh, glm::mat4& matrix) {
			glm::mat4 static_matrix {
				2.0f/SW,   0,   0,   0,
				0,  2.0f/SH,   0,   0,
				0,   0,   1,   0,
				-1,  -1,   0,   1,
			};

			level_shader.use();
			glUniformMatrix4fv(level_shader.uniform("uMatrix"), 1, GL_FALSE, glm::value_ptr(static_matrix));

			degrade_shader.use();
			glUniform2f(degrade_shader.uniform("uResolution"), rw, rh);
			glUniformMatrix4fv(degrade_shader.uniform("uMatrix"), 1, GL_FALSE, glm::value_ptr(matrix));

			__w = w;
			__h = h;
		});

		degrade_shader.use();
		const auto now_time = std::chrono::steady_clock::now();
		glUniform1f(degrade_shader.uniform("uTime"), std::chrono::duration_cast<std::chrono::duration<float>>(now_time - begin_time).count());

		glViewport(0, 0, SW, SH);

		// render
		level.draw(font8x8, text_writer, tileset, game_writer);
		game_writer.upload();
		text_writer.upload();

		// render
		pass_1.use();
		pass_1.clear();

		tileset.use();
		level_shader.use();
		game_buffer.draw();

		font8x8.use();
		text_buffer.draw();

		glViewport(0, 0, __w, __h);

		// apply a CRT-like effect and draw into back buffer
		pass_2.use();
		pass_2.clear();
		color_att.use();
		degrade_shader.use();
		blit_buffer.draw();

		SoundSystem::getInstance().update();
		Input::clear();

	});

    return EXIT_SUCCESS;
}
