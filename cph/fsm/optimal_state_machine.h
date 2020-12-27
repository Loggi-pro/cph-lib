#pragma once
#include "fsm_types.h"
//Combine pattern state machine
//functions as states
// 1. [Complexity]	(GOOD) Simple
// 1.1				(GOOD) Every state localized in every functions. And from this functions there are direct access to all members
// 1.1.				(NEUTRAL) Enumerating signals
// 2. [Reusable]	(GOOD) There are base class (generic processor with all transition logic)
// 2.1.				(BAD) Dont allow customize signature of each event handler
// 3. [Memory]		(GOOD) Require memory only on state pointer
// 4. [Time]		(NEUTRAL) Eliminated nested switches, only one switch. Second switch ->to function dispatch
// 5. [Hierarchy]	(BAD) Not hierarchical

namespace cph {
	//generic event processor


	namespace fsm {

		class Fsm;
		typedef void (Fsm::* State)(const Event&);

		class Fsm: FsmBase {
			public:

			private:
				State _currentState;
			protected:
				template <typename DerivedState>
				void transition(DerivedState target) {
					//(this->*_currentState)({ SIG_EXIT }); //exit action
					(this->*_currentState)(FsmBase::_ReservedEvents[SIG_EXIT]); //exit action
					_currentState = static_cast<State>(target);
					//(this->*_currentState)({ SIG_ENTRY }); //entry action
					(this->*_currentState)(FsmBase::_ReservedEvents[SIG_ENTRY]); //exit action
				}
			public:
				Fsm(State initial) : _currentState(initial) {}
				void init() {}
				void dispatch(const Event& ev) { (this->*_currentState)(ev); }
		};

		//concrete implementation
	}
}