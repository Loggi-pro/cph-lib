#pragma once
#include <cph\ioreg.h>

namespace cph {
	namespace Spi {
		namespace Private {
			template <uint8_t SIZE>
			class SpiFakeActions {
				public:
					void init() {
					}

			};

			typedef SpiFakeActions<255> FakeSpi_t;

			class SpiFakeHelper_t {

				public:
			};

		}
	}
}