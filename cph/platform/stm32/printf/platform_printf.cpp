#include <cph/printf.h>

cph::PrintF::printf_func_t cph::PrintF::printf_func = nullptr;

extern "C" {
	int _write (int fd, char* ptr, int len) {
		for (int i = 0; i < len; i++, ptr++) {
			cph::PrintF::printf_func(*ptr);
		}

		return len;
	}
}
