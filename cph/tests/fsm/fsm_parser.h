#pragma once
#include <cph/fsm.h>

class OptimalParser : public cph::fsm::Fsm {
	public:
		enum Signal {
			SIG_CHAR = cph::fsm::ReservedSignals::SIG_USER,
			SIG_STAR, SIG_SLASH
		};
	private:
		int _commentCount;
		//all actions coreesponding to code state
		void code(const cph::fsm::Event& ev) {
			switch (ev.sig) {
			case cph::fsm::SIG_EMPTY: break;

			case cph::fsm::SIG_ENTRY: break;

			case cph::fsm::SIG_EXIT: break;

			case SIG_SLASH:
				transition(&OptimalParser::slash);
				break;
			}
		}
		//all actions coreesponding to slash state
		void slash(const cph::fsm::Event& ev) {
			switch (ev.sig) {
			case cph::fsm::SIG_EMPTY: break;

			case cph::fsm::SIG_ENTRY: break;

			case cph::fsm::SIG_EXIT: break;

			case SIG_STAR:
				_commentCount += 2;
				transition(&OptimalParser::comment);
				break;

			case SIG_CHAR:
			case SIG_SLASH:
				transition(&OptimalParser::code);
				break;
			}
		}
		//all actions coreesponding to comment state
		void comment(const cph::fsm::Event& ev) {
			switch (ev.sig) {
			case cph::fsm::SIG_EMPTY: break;

			case cph::fsm::SIG_ENTRY: break;

			case cph::fsm::SIG_EXIT: break;

			case SIG_STAR:
				_commentCount++;
				transition(&OptimalParser::star);
				break;

			case SIG_CHAR:
				_commentCount++;
				break;

			case SIG_SLASH:
				_commentCount++;
				break;
			}
		}
		//all actions coreesponding to star state
		void star(const cph::fsm::Event& ev) {
			switch (ev.sig) {
			case cph::fsm::SIG_ENTRY: break;

			case cph::fsm::SIG_EXIT: break;

			case cph::fsm::SIG_EMPTY: break;

			case SIG_SLASH:
				_commentCount ++;
				transition(&OptimalParser::code);
				break;

			case SIG_CHAR:
				_commentCount ++;
				transition(&OptimalParser::comment);
				break;

			case SIG_STAR:
				_commentCount ++;
				transition(&OptimalParser::comment);
				break;
			}
		}
	public:
		OptimalParser() : cph::fsm::Fsm((cph::fsm::State) & OptimalParser::code),_commentCount(0) {}
		int commentCount()const { return _commentCount; }

};