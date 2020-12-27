#include "unity/unity_fixture.h"
#include "Buttons/buttons_port_fake.h"
#include "Buttons/buttons.h"
#include "timer_us_spy.h"
#include "Encoder/fake_encoder_port.h"


TEST_GROUP(ButtonTestGroup);


TEST_SETUP(ButtonTestGroup) {
	FakeSystemTimer_us_init(0, 0);
}

TEST_TEAR_DOWN(ButtonTestGroup) {

}

// NOTE: This tests seems little out of here cause id test jitter delay handling bu buttons module
TEST(ButtonTestGroup, ButtonJitterDelay) {
	ButtonsPortFake_setButtonAsPressed(BI_ENTER);
	FakeSystemTimer_us_delay(BUTTONS_SCAN_DELAY - 1);
	TEST_ASSERT_EQUAL(KEY_NONE, Buttons_keyCheck());

	bool buttonIsReaded = Buttons_scan();
	TEST_ASSERT_FALSE(buttonIsReaded);
	TEST_ASSERT_EQUAL(KEY_NONE, Buttons_keyCheck());

	FakeSystemTimer_us_delay(1);
	buttonIsReaded = Buttons_scan();
	TEST_ASSERT_TRUE(buttonIsReaded);
	TEST_ASSERT_EQUAL(KEY_ENTER, Buttons_keyCheck());
}


TEST_GROUP_RUNNER(ButtonTestGroup) {
	RUN_TEST_CASE(ButtonTestGroup, ButtonJitterDelay);

}