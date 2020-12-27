#pragma once
#include <cph/basic_usart.h>
#include <void/gsl.h>
#include <void/pair.h>
#include <cph/timer.h>
#include <void/delay.h>
#include <cph/async.h>
#define DFPLAYER_EQ_NORMAL 0
#define DFPLAYER_EQ_POP 1
#define DFPLAYER_EQ_ROCK 2
#define DFPLAYER_EQ_JAZZ 3
#define DFPLAYER_EQ_CLASSIC 4
#define DFPLAYER_EQ_BASS 5

enum DFPLAYER_DEVICE_FILECOUNT {
	DFPLAYER_DEVICE_FILECOUNT_U_DISK = 0x47,
	DFPLAYER_DEVICE_FILECOUNT_SD = 0x48,
	DFPLAYER_DEVICE_FILECOUNT_FLASH = 0x49
};

enum DFPLAYER_DEVICE_FILENUMBER {
	DFPLAYER_DEVICE_FILENUMBER_U_DISK = 0x4B,
	DFPLAYER_DEVICE_FILENUMBER_SD = 0x4C,
	DFPLAYER_DEVICE_FILENUMBER_FLASH = 0x4D
};

enum DFPLAYER_OUTPUT_DEVICE {
	DFPLAYER_OUTPUT_DEVICE_U_DISK = 1,
	DFPLAYER_OUTPUT_DEVICE_SD = 2,
	DFPLAYER_OUTPUT_DEVICE_AUX = 3,
	DFPLAYER_OUTPUT_DEVICE_SLEEP = 4,
	DFPLAYER_OUTPUT_DEVICE_FLASH = 5
};

//DFPLAYER_DEVICE_AUX =3,
//DFPLAYER_DEVICE_SLEEP =4,

#define DFPLAYER_RECEIVED_LENGTH 10
#define DFPLAYER_SEND_LENGTH 10

//#define _DEBUG

enum DFPLAYER_STATE {
	ERR_TIMEOUT = 0,
	ERR_WRONG_STACK = 1,
	DFPlayerCardInserted = 2,
	DFPlayerCardRemoved = 3,
	DFPlayerCardOnline = 4,
	DFPlayerPlayFinished = 5,
	DFPlayerError = 6,
	DFPlayerUSBInserted = 7,
	DFPlayerUSBRemoved = 8,
	DFPlayerUSBOnline = 9,
	DFPlayerCardUSBOnline = 10,
	DFPlayerFeedBack = 11,
	DFPlayerReply = 12 //Nothing to do with this
};


#define Busy 1
#define Sleeping 2
#define SerialWrongStack 3
#define CheckSumNotMatch 4
#define FileIndexOut 5
#define FileMismatch 6
#define Advertise 7


namespace cph {
	template <class TRANSPORT>
	class DFRobotDFPlayerMini {
			//
			enum ADDRESS {
				STACK_HEADER = 0,
				STACK_VERSION = 1,
				STACK_LENGTH = 2,
				STACK_COMMAND = 3,
				STACK_ACK = 4,
				STACK_PARAMETER = 5,
				STACK_CHECKSUMM = 7,
				STACK_END = 9
			};
			//
			vd::chrono::milli_t _timeOutDuration = 200;
			enum {IDLE, WAIT_ACK, COMPLETE} _state = IDLE;
			uint8_t _received[DFPLAYER_RECEIVED_LENGTH];
			uint8_t _buffer[DFPLAYER_SEND_LENGTH] = { 0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF };
			uint8_t _receivedIndex = 0;
			DFPLAYER_STATE _playerCmd;
			uint16_t _playerArg;
			cph::timer_t _timerTimeout;

			bool _sendCommand(uint8_t command, uint16_t argument = 0) {
				_buffer[STACK_COMMAND] = command;
				uint16ToArray(argument, _buffer + STACK_PARAMETER);
				uint16ToArray(calculateCheckSum(_buffer), _buffer + STACK_CHECKSUMM);
				return TRANSPORT::writeData(vd::gsl::span<uint8_t>(_buffer));
			}
			void enableACK() {
				_buffer[STACK_ACK] = 0x01;
			}
			void disableACK() {
				_buffer[STACK_ACK] = 0x00;
			}
			void uint16ToArray(uint16_t value, uint8_t* array) {
				*array = vd::hiByte(value);
				*(array + 1) = vd::lowByte(value);
			}
			uint16_t arrayToUint16(uint8_t* array) {
				uint16_t value = *array;
				value <<= 8;
				value += *(array + 1);
				return value;
			}
			uint16_t calculateCheckSum(uint8_t* buffer) {
				uint16_t sum = 0;

				for (int i = STACK_VERSION; i < STACK_CHECKSUMM; i++) {
					sum += buffer[i];
				}

				return -sum;
			}



			vd::pair<DFPLAYER_STATE, uint8_t> parseStack() {
				uint8_t rxCommand = _received[STACK_COMMAND];

				if (rxCommand == 0x41) {
					return {DFPlayerReply, 0};
				}

				//_rxCommand = rxCommand;
				uint8_t arg = arrayToUint16(_received + STACK_PARAMETER);

				switch (rxCommand) {
				case 0x3D:
					return {DFPlayerPlayFinished, arg};
					break;
				case 0x3F:
					if (arg & 0x01) {
						return {DFPlayerUSBOnline, arg};
					} else if (arg & 0x02) {
						return {DFPlayerCardOnline, arg};
					} else if (arg & 0x03) {
						return {DFPlayerCardUSBOnline, arg};
					}

					break;

				case 0x3A:
					if (arg & 0x01) {
						return {DFPlayerUSBInserted, arg};
					} else if (arg & 0x02) {
						return {DFPlayerCardInserted, arg};
					}

					break;

				case 0x3B:
					if (arg & 0x01) {
						return {DFPlayerUSBRemoved, arg};
					} else if (arg & 0x02) {
						return {DFPlayerCardRemoved, arg};
					}

					break;

				case 0x40:
					return {DFPlayerError, arg};
					break;
				case 0x3C:
				case 0x3E:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x45:
				case 0x46:
				case 0x47:
				case 0x48:
				case 0x49:
				case 0x4B:
				case 0x4C:
				case 0x4D:
				case 0x4E:
				case 0x4F:
					return {DFPlayerFeedBack, arg};
					break;
				default:
					return {ERR_WRONG_STACK, 0};
					break;
				}

				return {ERR_WRONG_STACK, 0};
			}
			bool validateStack() {
				return calculateCheckSum(_received) == arrayToUint16(_received + STACK_CHECKSUMM);
			}

			cph::TVoidResultAsync _sendAndWait(uint8_t cmd, uint16_t arg = 0) {
				switch (_state) {
				case IDLE:
					_receivedIndex = 0;

					if (!_timerTimeout.isElapsed(1_s)) { break; } //must be 200ms before any comand

					if (_sendCommand(cmd, arg)) {
						_state = WAIT_ACK;
						_timerTimeout.start();
					}

					break;

				case WAIT_ACK: {
					if (_timerTimeout.isElapsed(_timeOutDuration)) {
						_state = IDLE;
						_playerCmd = ERR_TIMEOUT;
						_playerArg = 0;
						return TAsyncState::E_TIMEOUT;
					}

					auto res = handleTransport();

					if (!res) { break; }

					_timerTimeout.start(); //сбрасываем таймер
					_state = IDLE;

					if (res.isFail() || !validateStack()) {
						_playerCmd = ERR_WRONG_STACK;
						_playerArg = 0;
						return TAsyncState::E_ERR;
					}

					auto parseResult = parseStack();
					_playerCmd = parseResult.first;
					_playerArg = parseResult.second;
					//[_playerCmd, _playerArg]
					return cph::TVoidResultAsync(TAsyncState::S_DONE);
					break;
				}

				default:
					break;
				}

				return TAsyncState::NO_RESULT;
			}
		public:


			//9600 speed
			cph::TVoidResultAsync init(bool isACK = true) {
				static bool initStarted = false;

				if (!initStarted) {
					if (!_timerTimeout.isElapsed(2_s)) {return TAsyncState::NO_RESULT;}

					initStarted = true;

					if (isACK) {
						enableACK();
					} else {
						disableACK();
					}

					TRANSPORT::reset();
				}

				//init started here
				if (!reset()) { return TAsyncState::NO_RESULT; }

				bool res = (_playerCmd == DFPlayerCardOnline) || (_playerCmd == DFPlayerUSBOnline) || !isACK;
				return res ? TAsyncState::S_DONE : TAsyncState::E_ERR;
			}
			cph::TVoidResultAsync handleTransport() {
				uint8_t byte;

				if (!TRANSPORT::readByte(&byte)) { return TAsyncState::NO_RESULT; }

				if (_receivedIndex == 0) {
					_received[STACK_HEADER] = byte;

					if (_received[STACK_HEADER] == 0x7E) {
						_receivedIndex++;
					}
				} else {
					_received[_receivedIndex] = byte;

					switch (_receivedIndex) {
					case STACK_VERSION:
						if (_received[_receivedIndex] != 0xFF) {
							//_resetState(ERR_WRONG_STACK);
							return cph::TVoidResultAsync(TAsyncState::E_ERR);
						}

						break;

					case STACK_LENGTH:
						if (_received[_receivedIndex] != 0x06) {
							//_resetState(ERR_WRONG_STACK);
							return cph::TVoidResultAsync(TAsyncState::E_ERR);
						}

						break;

					case STACK_END:
						if (_received[_receivedIndex] != 0xEF) {
							//_resetState(ERR_WRONG_STACK);
							return cph::TVoidResultAsync(TAsyncState::E_ERR);
						} else {
							return cph::TVoidResultAsync(TAsyncState::S_DONE);
						}

						break;

					default:
						break;
					}

					_receivedIndex++;
				}

				return TAsyncState::NO_RESULT;
			}
			inline vd::pair<DFPLAYER_STATE, uint8_t> readType() {
				return {_playerCmd, _playerArg};
			}
			cph::TVoidResultAsync next() {
				return _sendAndWait(0x01);
			}
			cph::TVoidResultAsync previous() {
				return _sendAndWait(0x02);
			}
			cph::TVoidResultAsync play(int fileNumber = 1) {
				return _sendAndWait(0x03, fileNumber);
			}
			cph::TVoidResultAsync volumeUp() {
				return _sendAndWait(0x04);
			}
			cph::TVoidResultAsync volumeDown() {
				return _sendAndWait(0x05);
			}
			cph::TVoidResultAsync volume(uint8_t volume) {
				return _sendAndWait(0x06, volume);
			}
			cph::TVoidResultAsync EQ(uint8_t eq) {
				return _sendAndWait(0x07, eq);
			}
			cph::TVoidResultAsync loop(int fileNumber) {
				return _sendAndWait(0x08, fileNumber);
			}
			cph::TVoidResultAsync outputDevice(DFPLAYER_OUTPUT_DEVICE device) {
				return _sendAndWait(0x09, device);
				vd::delay(200_ms);
			}
			cph::TVoidResultAsync sleep() {
				return _sendAndWait(0x0A);
			}
			cph::TVoidResultAsync reset() {
				return _sendAndWait(0x0C);
			}
			cph::TVoidResultAsync start() {
				return _sendAndWait(0x0D);
			}
			cph::TVoidResultAsync pause() {
				return _sendAndWait(0x0E);
			}
			cph::TVoidResultAsync playFolder(uint8_t folderNumber, uint8_t fileNumber) {
				return _sendAndWait(0x0F, vd::makeWord(fileNumber, folderNumber));
			}
			cph::TVoidResultAsync outputSetting(bool enable, uint8_t gain) {
				return _sendAndWait(0x10, gain, enable);
			}
			cph::TVoidResultAsync enableLoopAll() {
				return _sendAndWait(0x11, 0x01);
			}
			cph::TVoidResultAsync disableLoopAll() {
				return _sendAndWait(0x11, 0x00);
			}
			cph::TVoidResultAsync playMp3Folder(int fileNumber) {
				return _sendAndWait(0x12, fileNumber);
			}
			cph::TVoidResultAsync advertise(int fileNumber) {
				return _sendAndWait(0x13, fileNumber);
			}
			cph::TVoidResultAsync playLargeFolder(uint8_t folderNumber, uint16_t fileNumber) {
				return _sendAndWait(0x14, (((uint16_t)folderNumber) << 12) | fileNumber);
			}
			cph::TVoidResultAsync stopAdvertise() {
				return _sendAndWait(0x15);
			}
			cph::TVoidResultAsync stop() {
				return _sendAndWait(0x16);
			}
			cph::TVoidResultAsync loopFolder(int folderNumber) {
				return _sendAndWait(0x17, folderNumber);
			}
			cph::TVoidResultAsync randomAll() {
				return _sendAndWait(0x18);
			}
			cph::TVoidResultAsync enableLoop() {
				return _sendAndWait(0x19, 0x00);
			}
			cph::TVoidResultAsync disableLoop() {
				return _sendAndWait(0x19, 0x01);
			}
			cph::TVoidResultAsync enableDAC() {
				return _sendAndWait(0x1A, 0x00);
			}
			cph::TVoidResultAsync disableDAC() {
				return _sendAndWait(0x1A, 0x01);
			}
			cph::TResultAsync<uint8_t> readState() {
				auto res = _sendAndWait(0x42);

				if (!res) { return TAsyncState::NO_RESULT; }

				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
			cph::TResultAsync<uint8_t> readVolume() {
				auto res = _sendAndWait(0x43);

				if (!res) { return TAsyncState::NO_RESULT; }

				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
			cph::TResultAsync<uint8_t> readEQ() {
				auto res = _sendCommand(0x44);

				if (!res) { return TAsyncState::NO_RESULT; }

				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
			cph::TResultAsync<uint8_t> readFileCounts(DFPLAYER_DEVICE_FILECOUNT device) {
				auto res = _sendAndWait(device);

				if (!res) { return TAsyncState::NO_RESULT; }

				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
			cph::TResultAsync<uint8_t> readCurrentFileNumber(DFPLAYER_DEVICE_FILENUMBER device) {
				auto res = _sendAndWait(device);

				if (!res) { return TAsyncState::NO_RESULT; }

				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
			cph::TResultAsync<uint8_t> readFileCountsInFolder(int folderNumber) {
				auto res = _sendAndWait(0x4E, folderNumber);
				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
			cph::TResultAsync<uint8_t> readFolderCounts() {
				auto res = _sendAndWait(0x4F);
				const auto [command, arg] = readType();

				if (command == DFPlayerFeedBack) {
					return arg;
				} else {
					return TAsyncState::E_ERR;
				}
			}
	};

}

