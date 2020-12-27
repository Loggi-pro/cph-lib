#include "unity/unity_fixture.h"

#include "keyboard.h"
#include "timer_us_spy.h"

bool(*Buttons_scan_storage)(void);
TKeyValue(*Buttons_keyCheck_storage)(void);


static bool _isButtonScaned = false;
static TKeyValue _keyValue = KEY_UNKNOWN;

bool Buttons_scan_substitution(void){
	return _isButtonScaned;
}

TKeyValue Buttons_keyCheck_substitutuion(void){
	return _keyValue;
}



TEST_GROUP(KeyboardTestGroup);

static u16 _encoderTempValue = 0;
TKeyEvent _ev;
static void keyboardScanUntilReaded(void) {
	while (!Keyboard_getEvent(&_ev)) {
		continue;
	}
}





TEST_SETUP(KeyboardTestGroup) {
	// Save function pointer
	Buttons_scan_storage = Buttons_scan;
	Buttons_keyCheck_storage = Buttons_keyCheck;

	// Substitute function pointers
	Buttons_scan = Buttons_scan_substitution;
	Buttons_keyCheck = Buttons_keyCheck_substitutuion;

	_isButtonScaned = false;
	_keyValue = KEY_UNKNOWN;

	_ev.eventType = NO_KEY_EVENT;
	_ev._value = KEY_NONE;

	FakeSystemTimer_us_init(0, 0);
	Keyboard_Init();
	Encoder_initialize(0, 0, 100, 1);
}

TEST_TEAR_DOWN(KeyboardTestGroup) {
	// Restore function pointers
	Buttons_scan = Buttons_scan_storage;
	Buttons_keyCheck = Buttons_keyCheck_storage;
}




TEST(KeyboardTestGroup, NothingPushed) {
	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));
	TEST_ASSERT_EQUAL(NO_KEY_EVENT, _ev.eventType);
}




TEST(KeyboardTestGroup, OnKeyDown) {

	_isButtonScaned = true;
	_keyValue = KEY_ENTER;

	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));
	TEST_ASSERT_EQUAL(KEY_ENTER, _ev._value);
	TEST_ASSERT_EQUAL(ON_KEY_DOWN, _ev.eventType);

	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev)); 
}

TEST(KeyboardTestGroup, OnKeyUp) {

	_isButtonScaned = true;
	_keyValue = KEY_ENTER;
	Keyboard_getEvent(&_ev);

	_isButtonScaned = true;
	_keyValue = KEY_NONE;
	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));
 	TEST_ASSERT_EQUAL(KEY_ENTER, _ev._value);
 	TEST_ASSERT_EQUAL(ON_KEY_UP, _ev.eventType);
}

TEST(KeyboardTestGroup, OnKeyLong) {

	_isButtonScaned = true;
	_keyValue = KEY_ENTER;
	Keyboard_getEvent(&_ev);

	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));

	FakeSystemTimer_us_delay(KEY_LONG_DELAY - 1);
	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));  // Taking into consideration On Key Sequential event
	TEST_ASSERT_NOT_EQUAL(ON_KEY_LONG, _ev.eventType);  
	FakeSystemTimer_us_delay(1);

	// Checking On Long Event
	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));
	TEST_ASSERT_EQUAL(KEY_ENTER, _ev._value);
	TEST_ASSERT_EQUAL(ON_KEY_LONG, _ev.eventType);


	FakeSystemTimer_us_delay(1);
	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));
}

TEST(KeyboardTestGroup, OnKeySequetial) {

	_isButtonScaned = true;
	_keyValue = KEY_ENTER;
	Keyboard_getEvent(&_ev);

	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));

	FakeSystemTimer_us_delay(KEY_SEQUENTAL_DELAY_FIRST - 1);
	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));
	FakeSystemTimer_us_delay(1);

	// Checking First Sequential event
	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));
	TEST_ASSERT_EQUAL(KEY_ENTER, _ev._value);
	TEST_ASSERT_EQUAL(ON_KEY_SEQUENTIAL, _ev.eventType);

	// Checking Next Sequential events  1
	FakeSystemTimer_us_delay(KEY_SEQUENTAL_DELAY_PERIOD - 1);
	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));
	FakeSystemTimer_us_delay(1);

	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));
	TEST_ASSERT_EQUAL(KEY_ENTER, _ev._value);
	TEST_ASSERT_EQUAL(ON_KEY_SEQUENTIAL, _ev.eventType);


	// .....  2
	FakeSystemTimer_us_delay(KEY_SEQUENTAL_DELAY_PERIOD - 1);
	TEST_ASSERT_FALSE(Keyboard_getEvent(&_ev));
	FakeSystemTimer_us_delay(1);

	TEST_ASSERT_TRUE(Keyboard_getEvent(&_ev));
	TEST_ASSERT_EQUAL(KEY_ENTER, _ev._value);
	TEST_ASSERT_EQUAL(ON_KEY_SEQUENTIAL, _ev.eventType);
}


TEST_GROUP_RUNNER(KeyboardTestGroup) {
	RUN_TEST_CASE(KeyboardTestGroup, NothingPushed);
	RUN_TEST_CASE(KeyboardTestGroup, OnKeyDown);
	RUN_TEST_CASE(KeyboardTestGroup, OnKeyUp);
	RUN_TEST_CASE(KeyboardTestGroup, OnKeyLong);
	RUN_TEST_CASE(KeyboardTestGroup, OnKeySequetial);
}
