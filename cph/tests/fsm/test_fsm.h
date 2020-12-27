#pragma once
#include <unity/unity_fixture_classes.h>
#include "fsm_parser.h"
class CphTest_FSM : UnityTestClass<CphTest_FSM> {
		static inline OptimalParser parser = OptimalParser();
	public:
		static void setup() {
			parser.init();
		}
		static void tearDown() {
		}


		static void newTest() {
			parser.dispatch({OptimalParser::SIG_CHAR });
			parser.dispatch({OptimalParser::SIG_SLASH }); //open 1
			parser.dispatch({OptimalParser::SIG_STAR });//2
			TEST_ASSERT_EQUAL(2, parser.commentCount());
			parser.dispatch({OptimalParser::SIG_CHAR }); //3
			TEST_ASSERT_EQUAL(3, parser.commentCount());
			parser.dispatch({OptimalParser::SIG_CHAR }); //4
			TEST_ASSERT_EQUAL(4, parser.commentCount());
			parser.dispatch({OptimalParser::SIG_SLASH }); //5
			TEST_ASSERT_EQUAL(5, parser.commentCount());
			parser.dispatch({OptimalParser::SIG_STAR });//close 6
			TEST_ASSERT_EQUAL(6, parser.commentCount());
			parser.dispatch({OptimalParser::SIG_SLASH }); //7
			TEST_ASSERT_EQUAL(7, parser.commentCount());
			parser.dispatch({ OptimalParser::SIG_CHAR }); //7
			TEST_ASSERT_EQUAL(7, parser.commentCount());
		}

		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_FSM, newTest);
		}
};