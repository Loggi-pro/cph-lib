#pragma once
#include <cph/wake/wake.h>
#include <cph/timer.h>
#include <void/chrono.h>
#include <void/packed.h>
#include <cph/containers/buffer.h>

enum TWakeCommand { CMD_CONNECT = 0, CMD_STANDBY = 1, VARIABLE_GET = 2, VARIABLE_SET = 3, LAST_COMMAND };

enum TWakeCommandAnswer {
	ANSWER_CMD_RESULT = 0,
	ANSWER_GET_VARIABLE_RESULT = 1,
	ANSWER_SET_VARIABLE_RESULT = 2
};

enum TWakeDataAnswer {
	DATA_ANSWER_CANCEL = 0,
	DATA_ANSWER_OK = 1,
	DATA_ANSWER_PROGRESS = 2,
	DATA_ANSWER_NOT_SUPPORT = 3
};

//enum TVariableID { SETTINGS = 1 };

template <typename T>
struct TWakeVariableSetup {
	uint8_t id;
	uint8_t padding; //padding
	T variable;
};

PACKED(struct TWakeVariableSetupAnswer {
	uint8_t id;
	uint8_t result;
});

PACKED(struct TWakeVariableQuery {
	uint8_t id;
	uint8_t padding;
});

template <typename T>
struct TWakeVariableQueryAnswer {
	uint8_t id;
	uint8_t result;
	T variable;
};




namespace cph {
	namespace Private {

		template <class TParentController>
		class TConnectController {
				enum TConnectState { DISCONNECTED, CONNECTED };
				cph::timer_t _lastReceivedTime;
				TConnectState _currentState;
				vd::chrono::milli_t _timeout;
				bool _wasReceived;
				TParentController* self() {
					return static_cast<TParentController*>(this);
				}

			protected:
				void keepInConnection() {
					_lastReceivedTime.start();
				}

			public:
				template <class Period>
				constexpr explicit TConnectController(vd::chrono::any_t<Period> timeout) : _currentState(
					    DISCONNECTED),
					_timeout(timeout.count()), _wasReceived(false) {
				}
				void reset() {
					_lastReceivedTime.start();
					_currentState = DISCONNECTED;
					_wasReceived = false;
				}

				void poll() {
					if (_currentState != CONNECTED) { return; }

					if (_wasReceived && _lastReceivedTime.isElapsed(_timeout)) {
						_currentState = DISCONNECTED;
						self()->_onDisconnectAction();
					}
				}
				void handlePacket(const vd::gsl::span<uint8_t>& buff) {
					keepInConnection();

					if (_currentState == DISCONNECTED) {
						_wasReceived = true;
						_currentState = CONNECTED;
						self()->_onConnectAction();
					}

					self()->_onPacketReceiveAction(buff);
				}
				bool isConnected()const {
					return _currentState == CONNECTED;
				}
		};
	}
}

typedef bool(*onAnswerCb_t)(const vd::gsl::span<uint8_t>& data);
template <typename T>
using getter_fcn = const T & (*)();
template <typename T>
using setter_fcn = bool(*)(const T&);

template <uint8_t id, typename T, getter_fcn<T> getter, setter_fcn<T> setter>
struct TConfig {
	typedef T type;
	static constexpr uint8_t getId() {
		return id;
	}
	static constexpr  auto getGetter() {
		return getter;
	}
	static constexpr  auto getSetter() {
		return setter;
	}
};


template<typename T>
TWakeVariableSetup<T> setupVariable(uint8_t id, const T& value,
                                    const vd::gsl::span<uint8_t>& buff) {
	TWakeVariableSetup<T> setup;
	setup->id = id;
	setup->variable = value;
	return setup;
}



template <typename ...TConfig>
struct getVariable {};



template<typename TConfig>
struct getVariable<TConfig> { //for single arguments

	static vd::gsl::span<uint8_t> _run(const vd::gsl::span<uint8_t>& buff) {
		typedef typename TConfig::type variable_t;
		TWakeVariableQuery* header = (TWakeVariableQuery*)buff.data();

		if (header->id != TConfig::getId()) { return buff.subspan(0, 0); }

		TWakeVariableQueryAnswer<variable_t> answer;
		answer.id = TConfig::getId();
		VOID_STATIC_ASSERT(sizeof(TWakeVariableQueryAnswer <variable_t>) == 2 + sizeof(variable_t));

		if (TConfig::getGetter() != nullptr) {
			answer.result = DATA_ANSWER_OK;
			answer.variable = TConfig::getGetter()(); //call function to get data
		} else {
			answer.result = DATA_ANSWER_NOT_SUPPORT;
		}

		return cph::TWakeProtocol::makePacket(buff, cph::TWakeInfo::weBROADCAST,
		                                      ANSWER_GET_VARIABLE_RESULT,
		                                      vd::gsl::span<uint8_t>(answer));
	}

	static void handle(const vd::gsl::span<uint8_t>& buff, onAnswerCb_t cb) {
		auto out = getVariable<TConfig>::_run(buff);

		if (cb) {
			cb(out);
		};
	}
};

template <typename T, typename T2, typename ...TConfigs>
struct getVariable<T, T2, TConfigs...> { // for multiple arguments
	static vd::gsl::span<uint8_t> _run(const vd::gsl::span<uint8_t>& buff) {
		auto out = getVariable<T>::_run(buff);

		if (out.size() == 0) { return getVariable<T2, TConfigs...>::_run(buff); } //try next in chain

		return out;
	}
	static void handle(const vd::gsl::span<uint8_t>& buff, onAnswerCb_t cb) {
		auto out = getVariable<T, T2, TConfigs...>::_run(buff);

		if (cb) {
			cb(out);
		};
	}

};

template <typename ...TConfig>
struct setVariable {};



template<typename TConfig>
struct setVariable<TConfig> { //for single arguments
	static vd::gsl::span<uint8_t> _run(const vd::gsl::span<uint8_t>& buff) {
		typedef typename TConfig::type variable_t;
		TWakeVariableSetup<variable_t>* header = (TWakeVariableSetup<variable_t>*)
		        buff.data();

		if (header->id != TConfig::getId()) { return buff.subspan(0, 0); }

		TWakeVariableSetupAnswer answer;
		answer.id = TConfig::getId();
		answer.result = DATA_ANSWER_CANCEL;

		if (TConfig::getSetter() != nullptr) {
			if (buff.size() - OFFSETOF(TWakeVariableSetup<variable_t>, variable) == sizeof(variable_t)) {
				variable_t* ptr = &header->variable;

				if (TConfig::getSetter()(*ptr)) {
					answer.result = DATA_ANSWER_OK;
				}
			}
		}

		return cph::TWakeProtocol::makePacket(buff, cph::TWakeInfo::weBROADCAST, ANSWER_SET_VARIABLE_RESULT,
		                                      vd::gsl::span<uint8_t>(answer));
	}

	static void handle(const vd::gsl::span<uint8_t>& buff, onAnswerCb_t cb) {
		auto out = setVariable<TConfig>::_run(buff);
		cb(out);
	}
};

template <typename T, typename T2, typename ...TConfig>
struct setVariable<T, T2, TConfig...> { //for multiple arguments
	static vd::gsl::span<uint8_t> _run(const vd::gsl::span<uint8_t>& buff) {
		auto out = setVariable<T>::_run(buff);

		if (out.size() == 0) { return setVariable<T2, TConfig...>::_run(buff); }

		return out;
	}
	static void handle(const vd::gsl::span<uint8_t>& buff, onAnswerCb_t cb) {
		auto out = setVariable<T, T2, TConfig...>::_run(buff);
		cb(out);
	}

};




template <class PARENT, class ...TConfig>
class TWakeController: public cph::Private::TConnectController
	<TWakeController<PARENT, TConfig...>> {
		typedef  cph::Private::TConnectController < TWakeController < PARENT, TConfig...>> TBase;
	private:
		friend class cph::Private::TConnectController<TWakeController<PARENT, TConfig...>>; //for crtp
		vd::gsl::span<uint8_t> _buffer;
		onAnswerCb_t _cbOnAnswer;
		PARENT* self() {
			return static_cast<PARENT*>(this);
		}

		bool _onPacketReceiveAction(const vd::gsl::span<uint8_t>& buff) {
			cph::TWakePacket packet(buff);
			TWakeCommand cmd = (TWakeCommand)packet.cmd();
			bool res = true;

			switch (cmd) {
			case CMD_CONNECT: {
				uint8_t data[] = { DATA_ANSWER_OK };
				auto out = cph::TWakeProtocol::makePacket(buff, cph::TWakeInfo::weBROADCAST, ANSWER_CMD_RESULT,
				           data);

				if (_cbOnAnswer) { _cbOnAnswer(out); }

				break;
			}

			case CMD_STANDBY:
				break;

			case VARIABLE_GET: {
				getVariable<TConfig...>::handle(vd::gsl::span<uint8_t>(packet.data(), packet.count()), _cbOnAnswer);
				break;
			}

			case VARIABLE_SET: {
				setVariable<TConfig...>::handle(vd::gsl::span<uint8_t>(packet.data(), packet.count()), _cbOnAnswer);
				break;
			}

			default: {
				uint8_t data[] = { DATA_ANSWER_NOT_SUPPORT };
				auto out = cph::TWakeProtocol::makePacket(buff, cph::TWakeInfo::weBROADCAST,
				           ANSWER_CMD_RESULT, data);

				if (_cbOnAnswer) { _cbOnAnswer(out); }

				break;
			}
			}

			return res;
		}
		void _onConnectAction() {
			self()->_onConnectAction();
		}
		void _onDisconnectAction() {
			self()->_onDisconnectAction();
		}

	public:


		void setOnAnswer(onAnswerCb_t cb) { _cbOnAnswer = cb; }
		template <uint8_t BUFFER_SIZE, class literalTime>
		explicit TWakeController(onAnswerCb_t cb,
		                         const cph::TWakeAddress& addr, literalTime connectTime,
		                         cph::value_t<BUFFER_SIZE>) : TBase(connectTime),
			_cbOnAnswer(cb) {
			static uint8_t buffer[BUFFER_SIZE];
			_buffer = vd::gsl::span(buffer);
			cph::TWakeProtocol::init<BUFFER_SIZE>(addr);
		}
		bool poll() {
			TBase::poll();
			bool res = cph::TWakeProtocol::readPacketAsync(_buffer);

			if (res) {
				//TUsart::writeByte('1');
				TBase::handlePacket(_buffer);
			}

			return res;
		}
};


