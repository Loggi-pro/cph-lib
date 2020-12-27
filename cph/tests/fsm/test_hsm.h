#pragma once
#include <unity/unity_fixture_classes.h>
#include "hsm_parser.h"



class CphTest_HSM : UnityTestClass<CphTest_HSM> {
		static inline HsmParser parser = HsmParser();
	public:
		static void setup() {
			parser.init();
		}
		static void tearDown() {
		}


		static void newTest() {
			parser.dispatch({ HsmParser::SIG_CHAR });
			parser.dispatch({ HsmParser::SIG_SLASH }); //open 1
			parser.dispatch({ HsmParser::SIG_STAR });	//2
			TEST_ASSERT_EQUAL(2, parser.commentCount());
			parser.dispatch({ HsmParser::SIG_CHAR }); //3
			TEST_ASSERT_EQUAL(3, parser.commentCount());
			parser.dispatch({ HsmParser::SIG_CHAR }); //4
			TEST_ASSERT_EQUAL(4, parser.commentCount());
			parser.dispatch({ HsmParser::SIG_SLASH }); //5
			TEST_ASSERT_EQUAL(5, parser.commentCount());
			parser.dispatch({ HsmParser::SIG_STAR }); //close 6
			TEST_ASSERT_EQUAL(6, parser.commentCount());
			parser.dispatch({ HsmParser::SIG_SLASH }); //7
			TEST_ASSERT_EQUAL(7, parser.commentCount());
			parser.dispatch({ HsmParser::SIG_CHAR }); //7
			TEST_ASSERT_EQUAL(7, parser.commentCount());
		}

		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_HSM, newTest);
		}
};

