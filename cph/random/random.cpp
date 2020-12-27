#include <cph/random.h>
#include <cph/adc.h>
#include <cph/system_counter.h>


void Random::init() {
}




uint32_t Random::rand(uint32_t min, uint32_t max) {
	cph::Adc::startSingleConversion();

	while (!cph::Adc::resultReady()) { continue; }

	return (rnd_math(cph::Adc::read() + time_rnd()) % (max - min) + min);
}




void Random::poll() {
}

