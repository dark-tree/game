#pragma once

#include "const.hpp"

struct PlatformKeyStateScope {
	enum KeyStateEnum : unsigned int {
		UP    = 0b00,
		DOWN  = 0b01,
		TYPED = 0b11,
	};
};

struct PlatformKeyScope {
	enum KeyEnum : unsigned int {
		UNDEF = 0,

		LEFT   = 37,
		RIGHT  = 39,
		UP     = 38,
		DOWN   = 40,
		SPACE  = 32,
		ESCAPE = 27,
		TAB    = 9,
		ENTER  = 13,
		W      = 87,
		A      = 65,
		D      = 68,
		B      = 66,
	};
};

// export enums
using Key = PlatformKeyScope::KeyEnum;
using KeyState = PlatformKeyStateScope::KeyStateEnum;

// event callback types
using PlatformKeyEventCallback = void(*)(Key);
using PlatformMouseEventCallback = void(*)(float, float);
using PlatformLoopCallback = void(*)();

#if defined(__EMSCRIPTEN__)

	#include <emscripten.h>
	#include <emscripten/html5.h>
	#include <GLES3/gl3.h>

	#define EXPORTED_NATIVE EMSCRIPTEN_KEEPALIVE

	namespace __impl {

		/// translates the javascript's key code into key enum
		inline Key __platform_translate_key(const char* key) {

			if (strcmp(key, "ArrowLeft") == 0) return Key::LEFT;
			if (strcmp(key, "ArrowRight") == 0) return Key::RIGHT;
			if (strcmp(key, "ArrowUp") == 0) return Key::UP;
			if (strcmp(key, "ArrowDown") == 0) return Key::DOWN;
			if (strcmp(key, "Space") == 0) return Key::SPACE;
			if (strcmp(key, "Escape") == 0) return Key::ESCAPE;
			if (strcmp(key, "Tab") == 0) return Key::TAB;
			if (strcmp(key, "Enter") == 0) return Key::ENTER;
			if (strcmp(key, "KeyW") == 0) return Key::W;
			if (strcmp(key, "KeyA") == 0) return Key::A;
			if (strcmp(key, "KeyD") == 0) return Key::D;
			if (strcmp(key, "KeyB") == 0) return Key::B;

			return Key::UNDEF;
		}

		inline EM_BOOL __platform_keydown_handler(int type, const EmscriptenKeyboardEvent* event, void* userdata) {
			reinterpret_cast<PlatformKeyEventCallback>(userdata)(__platform_translate_key(event->code));
			return true;
		}

		inline EM_BOOL __platform_keyup_handler(int type, const EmscriptenKeyboardEvent* event, void* userdata) {
			reinterpret_cast<PlatformKeyEventCallback>(userdata)(__platform_translate_key(event->code));
			return true;
		}

		inline EM_BOOL __platform_mousemove_handler(int type, const EmscriptenMouseEvent* event, void* userdata) {
			reinterpret_cast<PlatformMouseEventCallback>(userdata)(event->clientX, event->clientY);
			return true;
		}

		inline void __platform_write_string(std::string path, std::string data) {
			std::string code = "localStorage.setItem('" + path + "', '" + data + "');";
			emscripten_run_script(code.data());
		}

		inline std::string __platform_read_string(std::string path) {
			std::string code = "localStorage.getItem('" + path + "') || '';";
			return emscripten_run_script_string(code.data());
		}

	}

	[[noreturn]] inline void platform_exit(int code) {
		emscripten_force_exit(code);
	}

	inline void platform_set_keydown_callback(PlatformKeyEventCallback callback) {
		emscripten_set_keydown_callback(HTML_CANVAS, (void*) callback, false, __impl::__platform_keydown_handler);
	}

	inline void platform_set_keyup_callback(PlatformKeyEventCallback callback) {
		emscripten_set_keyup_callback(HTML_CANVAS, (void*) callback, false, __impl::__platform_keyup_handler);
	}

	inline void platform_get_canvas_element_size(int* width, int* height) {
		emscripten_get_canvas_element_size(HTML_CANVAS, width, height);
	}

	inline void platform_set_main_loop(PlatformLoopCallback callback, int fps) {
		emscripten_set_main_loop((em_callback_func) callback, fps, true);
	}

	inline void platform_init() {
		printf("Began 'Emscripten/WebGL' platform init...\n");

		EmscriptenWebGLContextAttributes attrs;
		attrs.alpha = false;
		attrs.depth = true;
		attrs.stencil = true;
		attrs.antialias = true;
		attrs.premultipliedAlpha = false;
		attrs.preserveDrawingBuffer = false;
		attrs.powerPreference = false;
		attrs.failIfMajorPerformanceCaveat = true;
		attrs.majorVersion = 2;
		attrs.minorVersion = 0;
		attrs.enableExtensionsByDefault = false;

		// create the context
		int context = emscripten_webgl_create_context(HTML_CANVAS, &attrs);

		if (context == 0) {
			printf("Failed to create WebGL context!\n");
			platform_exit(-1);
		}

		// and bind it the the current "thread"
		emscripten_webgl_make_context_current(context);
	}

	inline void platform_write(std::string path, std::string data) {
		std::string code = "localStorage.setItem('" + path + "', '" + data + "');";
		emscripten_run_script(code.data());
	}

	inline std::string platform_read(std::string path) {
		std::string code = "localStorage.getItem('" + path + "');";
		return emscripten_run_script_string(code.data());
	}

#elif defined(__linux__)

	// sudo apt-get install libopenal-dev

	#include <winx.h>
	#include <glad/glad.h>
	#include <sys/stat.h>

	#define EXPORTED_NATIVE

	namespace __impl {

		extern int __screen_width;
		extern int __screen_height;
		extern PlatformKeyEventCallback __keydown_callback;
		extern PlatformKeyEventCallback __keyup_callback;

		/// translates the system keycode into key enum
		inline Key __platform_translate_key(int key) {

			if (key == WXK_LEFT) return Key::LEFT;
			if (key == WXK_RIGHT) return Key::RIGHT;
			if (key == WXK_UP) return Key::UP;
			if (key == WXK_DOWN) return Key::DOWN;
			if (key == WXK_SPACE) return Key::SPACE;
			if (key == WXK_ESC) return Key::ESCAPE;
			if (key == WXK_TAB) return Key::TAB;
			if (key == WXK_ENTER) return Key::ENTER;
			if (key == 'W' || key == 'w') return Key::W;
			if (key == 'A' || key == 'a') return Key::A;
			if (key == 'D' || key == 'd') return Key::D;
			if (key == 'B' || key == 'b') return Key::B;

			return Key::UNDEF;
		}

		inline void __platform_close_handler() {
			exit(-1);
		}

		inline void __platform_keyboard_handler(int state, int keycode) {
			Key key = __platform_translate_key(keycode);

			if (state == WINX_PRESSED) {
				__keydown_callback(key);
			} else {
				__keyup_callback(key);
			}
		}

		inline void __platform_resize_handler(int width, int height) {
			__screen_width = width;
			__screen_height = height;
		}

		inline void __platform_write_string(std::string path, std::string data) {
			printf("Writing state 'local/%s'\n", path.c_str());

			mkdir("./local", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

			std::ofstream f;
			f.open("./local/" + path, std::ofstream::binary | std::ifstream::out);
			f.write(data.data(), data.size());
		}

		inline std::string __platform_read_string(std::string path) {
			printf("Reading state 'local/%s'\n", path.c_str());

			std::ifstream f;
			f.open("./local/" + path, std::ifstream::binary | std::ifstream::in);

			if (!f.is_open()) {
				return "";
			}

			f.seekg(0, std::ifstream::end);
			const auto size = f.tellg();
			f.seekg(0, std::ifstream::beg);

			std::string buffer;
			buffer.resize(size);
			f.read(buffer.data(), size);

			return buffer;
		}

	}

	[[noreturn]] inline void platform_exit(int code) {
		exit(code);
	}

	inline void platform_set_keydown_callback(PlatformKeyEventCallback callback) {
		__impl::__keydown_callback = callback;
	}

	inline void platform_set_keyup_callback(PlatformKeyEventCallback callback) {
		__impl::__keyup_callback = callback;
	}

	inline void platform_get_canvas_element_size(int* width, int* height) {
		*width = __impl::__screen_width;
		*height = __impl::__screen_height;
	}

	inline void platform_set_main_loop(PlatformLoopCallback callback, int fps) {
		while (true) {
			callback();

			winxSwapBuffers();
			winxPollEvents();
		}
	}

	inline void platform_init() {
		printf("Began 'Winx/OpenGL' platform init...\n");

		winxHint(WINX_HINT_VSYNC, WINX_VSYNC_ENABLED);

		if (!winxOpen(GAME_NATIVE_WIDTH, GAME_NATIVE_HEIGHT, GAME_TITLE)) {
			printf("Failed to create WINX context! %s\n", winxGetError());
			platform_exit(-1);
		}

		// use GLAD to load OpenGL functions
		gladLoadGL();

		__impl::__screen_width = GAME_NATIVE_WIDTH;
		__impl::__screen_height = GAME_NATIVE_HEIGHT;

		winxSetCloseEventHandle(__impl::__platform_close_handler);
		winxSetKeyboardEventHandle(__impl::__platform_keyboard_handler);
		winxSetResizeEventHandle(__impl::__platform_resize_handler);
	}

#endif

// shared
inline void platform_write_string(std::string path, std::string data) {
	__impl::__platform_write_string(path, data);
}

inline std::string platform_read_string(std::string path) {
	return __impl::__platform_read_string(path);
}