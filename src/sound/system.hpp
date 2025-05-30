#pragma once

#include <external.hpp>
#include <sound/debug.hpp>
#include <sound/buffer.hpp>
#include <sound/source.hpp>
#include <sound/listener.hpp>
#include <sound/group.hpp>

// to learn all the Audio api quirks
// see: https://emscripten.org/docs/porting/Audio.html

class SoundSystem {

	private:

		ALCdevice* device;
		ALCcontext* context;

		std::list<std::unique_ptr<SoundSource>> sources;

		SoundSystem() {
			device = alcOpenDevice(nullptr);

			if (device == nullptr) {
				fault("Sound system failed to start, unable to open device!\n");
			}

			context = alcCreateContext(device, nullptr);

			if (context == nullptr) {
				fault("Sound system failed to start, unable to create context!\n");
			}

			alcMakeContextCurrent(context);
			debug::openal::check_error("alcMakeContextCurrent");

			listener().position(0, 0).velocity(0, 0).gain(1.0f);

			printf("Sound system started!\n");
		}

		~SoundSystem() {
			// we can't close the Sound System as the buffers are stored in static arrays
			// and don't get deleted before Sound System closes itself...

			// alcMakeContextCurrent(nullptr);
			// alcDestroyContext(context);
			// alcCloseDevice(device);
			// printf("Sound system stopped!\n");
		}

	public:

		static SoundSystem& getInstance() {
			static SoundSystem system;
			return system;
		}

		void update() {
			std::list<std::unique_ptr<SoundSource>>::iterator iter = sources.begin();

			int count = sources.size();
			int freed = 0;

			if (count < 100) {
				return;
			}

			while (iter != sources.end()) {
				bool drop = (*iter)->shouldDrop();

				if (drop) {
					iter = sources.erase(iter);
					freed ++;
				} else {
					iter ++;
				}
			}

			printf("Cleanup up %d sounds out of %d total\n", freed, count);
		}

		SoundSource& add(const SoundGroup& group) {
			return add(group.pick());
		}

		SoundSource& add(const SoundBuffer& buffer) {
			return add(std::make_unique<SoundSource>(buffer));
		}

		SoundSource& add(std::unique_ptr<SoundSource>&& source) {
			sources.push_back(std::move(source));

			return *sources.back();
		}

		SoundListener listener() {
			return {};
		}

		void stop() {
			for (auto& source : sources) {
				source->drop();
			}
		}

};
