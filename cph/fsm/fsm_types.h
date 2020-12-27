#pragma once
namespace cph {
	namespace fsm {
		enum ReservedSignals {
			SIG_EMPTY = 0,
			SIG_ENTRY = 1, /* signal for coding entry actions */
			SIG_EXIT, /* signal for coding exit actions */
			SIG_INIT, /* signal for coding initial transitions */
			SIG_USER /* first signal that can be used in user code */
		};
		typedef uint8_t Signal;
		struct Event {
			Signal sig;
			//can be extended for other arguments
		};

		class FsmBase {
			protected:
				constexpr static Event const _ReservedEvents[] = {
					{ (cph::fsm::Signal)cph::fsm::SIG_EMPTY },
					{ (cph::fsm::Signal)cph::fsm::SIG_ENTRY },
					{ (cph::fsm::Signal)cph::fsm::SIG_EXIT },
					{ (cph::fsm::Signal)cph::fsm::SIG_INIT}
				};
		};
	}

}