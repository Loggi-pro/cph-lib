#include <unity/unity_fixture.h>
#include <cph/tests/tests.h>



int main(int argc, const char* argv[]) {
	UnityMain(argc, argv, &runCphTests);
}
