#pragma once
#include <cph/fsm.h>
class HsmParser : public cph::hsm::Hsm {
		uint32_t _comment_count;


	public:
		enum HsmParserSignals {
			SIG_SLASH = cph::fsm::ReservedSignals::SIG_USER, // user sisngals start with SIG_USER
			SIG_STAR, SIG_CHAR
		};
		HsmParser() : Hsm(cph::hsm::as_state(&HsmParser::initial)), _comment_count(0) {}
		uint32_t commentCount()const {
			return _comment_count;
		}

		constexpr static cph::fsm::Event const testEvent[] = {
			{HsmParser::SIG_SLASH}, {HsmParser::SIG_STAR}, {HsmParser::SIG_CHAR}
		};
		template <HsmParserSignals sig>
		constexpr static cph::fsm::Event const& getEvent() {
			return testEvent[sig - cph::fsm::ReservedSignals::SIG_USER];
		}

	private:
		constexpr cph::hsm::EventResult initial(const cph::fsm::Event*) {
			return eventTransition(cph::hsm::as_state(&HsmParser::code));
		}
		cph::hsm::EventResult code(const cph::fsm::Event* e) { //state handler
			switch (e->sig) {
			case SIG_SLASH: return eventTransition(&HsmParser::code_slash);

			case SIG_STAR: return eventHandled();

			case SIG_CHAR: return eventHandled();
			}

			return eventSuper(&Hsm::top);
		}
		/**/cph::hsm::EventResult code_slash(const cph::fsm::Event* e) {  //state handler
			switch (e->sig) {
			case SIG_SLASH: return eventTransition(&HsmParser::code);

			case SIG_STAR: return eventTransition(&HsmParser::comment);

			case SIG_CHAR: return eventTransition(&HsmParser::code);

			default:
				break;
			}

			return eventSuper(&HsmParser::code);
		}
		cph::hsm::EventResult comment(const cph::fsm::Event* e) { //state handler
			switch (e->sig) {
			case cph::fsm::SIG_ENTRY:
				_comment_count += 2;
				return eventHandled();

			case SIG_SLASH:
				_comment_count += 1;
				return eventHandled();

			case SIG_STAR:
				_comment_count += 1;
				return eventTransition(&HsmParser::comment_star);

			case SIG_CHAR:
				_comment_count += 1;
				return eventHandled();

			default:
				break;
			}

			return eventSuper(&Hsm::top);
		}
		/**/cph::hsm::EventResult comment_star(const cph::fsm::Event* e) {  //state handler
			switch (e->sig) {
			case SIG_SLASH:
				_comment_count += 1;
				return eventTransition(&HsmParser::code);

			case SIG_STAR:
				_comment_count += 1;
				return eventHandled();

			case SIG_CHAR:
				_comment_count += 1;
				return eventTransition(&HsmParser::comment);

			default:
				break;
			}

			return eventSuper(&HsmParser::comment);
		}

};


