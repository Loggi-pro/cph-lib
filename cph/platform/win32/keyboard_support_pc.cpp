#include <conio.h>
#include <process.h>

#include "buttons/buttons_port_fake.h"
#include "encoder/encoder.h"
#include "encoder/fake_encoder_port.h"
//#include "s74hc595_spy.h"

#include "delay.h"
#include <windows.h>
//IF GetKeyState = unresolver external symbol, then add User32.lib to linker dependencyes
struct TKeyArr {
	char key;
	ButtonIndex butInd;
};

// Actual shift register bits buttons connected to.


//CHARS MUST BE UPPER CASE (USE VIRTUAL-KEY TABLE)
//https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396

const u08 numOfButtons = 3;
TKeyArr keys[numOfButtons] = {
	{ VK_SPACE, BI_ENTER },
	{ 'S', BI_ESC },
	{ VK_DOWN, BI_ENCODER_CLICK },
};


bool isKeyPressed(char key) {
	if (GetKeyState(key) & 0x8000) {
		return true;
	} else { return false; }
}

char prevKey_pressed = 0;
char currentKey_pressed = 0;
void keyCheck(void* param) {
	while (true) {
		prevKey_pressed = currentKey_pressed;
		currentKey_pressed = 0;
		u08 res = 0;

		bool oneOfKeysIsPresed = false;
		for (int i = 0; i < numOfButtons; i++) {
			if (isKeyPressed(keys[i].key)) {
				oneOfKeysIsPresed = true;
				currentKey_pressed = keys[i].key;
				ButtonsPortFake_setButtonAsPressed(keys[i].butInd);
			}
		}
		if (!oneOfKeysIsPresed){
			ButtonsPortFake_setButtonAsPressed(BI_NONE);
		}

		if (isKeyPressed(VK_RIGHT)) {
			currentKey_pressed = VK_RIGHT;
		} else if (isKeyPressed(VK_LEFT)) {
			currentKey_pressed = VK_LEFT;
		}

		if (Encoder_isInited()) {
			if (currentKey_pressed == 0 && prevKey_pressed == VK_RIGHT) {
				FakeEncoderPort_turnRightHelper();
			}

			if (currentKey_pressed == 0 && prevKey_pressed == VK_LEFT) {
				FakeEncoderPort_turnLeftHelper();
			}
		}
	}
}

class TKeyboard_listener {
	public:
		TKeyboard_listener() {
			_beginthread(keyCheck, 0, NULL);
		}
};

static TKeyboard_listener listener;