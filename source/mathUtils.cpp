#include "mathUtils.h"

#include <random>
#include <chrono>

template<class T = std::mt19937, size_t N = T::state_size>
auto seededRandomEngine() -> typename std::enable_if<!!N, T>::type
{
	std::random_device rd;
	if (rd.entropy() != 0)
	{
		std::seed_seq seeds
		{
			rd(),
			rd(),
			rd(),
			rd(),
			rd(),
			rd(),
			rd(),
			rd(),
		};
		T seededEngine(seeds);
		return seededEngine;
	}
	srand(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	std::seed_seq seeds
	{
		static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
		static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count() >> 32),
		rand(),
		rand(),
		rand(),
		rand(),
		rand(),
		rand(),
	};
	T seededEngine(seeds);
	return seededEngine;
}

// return random number between mi and ma
int32_t random(int32_t minimum, int32_t maximum)
{
	static thread_local std::mt19937 engine(seededRandomEngine());

	// Distribution goes from 0 to TYPE_MAX by default
	static std::uniform_int_distribution<int32_t> distrInt;

	return (minimum + (distrInt(engine) % (maximum - minimum + 1)));
}

//Not the original code, because it's better
int getSin(uint8_t deg)
{
	return static_cast<int>(sin(deg * (M_PI / 0x80)) * 512.0);
}

int getCos(uint8_t deg)
{
	return static_cast<int>(cos(deg * (M_PI / 0x80)) * 512.0);
}

uint8_t getAtan(int x, int y)
{
	return static_cast<uint8_t>(atan2(-y, -x) * 0x80 / M_PI);
}
