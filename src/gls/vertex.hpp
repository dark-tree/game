#pragma once

namespace gls {

	struct Vert4f4b {

		float x, y;
		float u, v;
		uint8_t r, g, b, a;

		Vert4f4b(float x, float y, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: x(x), y(y), u(u), v(v), r(r), g(g), b(b), a(a) {}

	};

	struct TypeAsserts {
		static_assert (sizeof(Vert4f4b) == 4 * sizeof(float) + 4 * sizeof(uint8_t), "Vert4f4b is not of the correct size!");
	};

}