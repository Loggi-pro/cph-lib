#pragma once
#include <void\static_assert.h>
#include <void/assert.h>
#include <void/containers/ring_buffer.h>
#include <cph/async.h>
#include <void/atomic.h>
namespace cph {


	struct TWakeInfo {
		enum SpecialCodes { //Internal system codes
			FEND = 0xC0,	//start sequence
			FESC = 0xDB,	//escape sequence
			TFEND = 0xDC,	//esc-fend
			TFESC = 0xDD	//esc-fesc
		};
		enum Error {
			weNO_ERROR = 0,
			weFORMAT = 1,
			weOVERLOAD = 2,
			weADDR = 3,
			weCRC = 4
		};
		enum CmdCode { //Standart cmd codes
			weCMD_NOP = 0x00,	//NOP
			weCMD_ERR = 0x01,	//ERROR
			weCMD_ECHO = 0x02,	//Echo reply this packet
			weCMD_INFO = 0x03,	//Reply info about device
		};
		enum ArgCode { //Standart error codes arguments - send to remote device
			weARG_ERR_NO = 0x00,//Command complete
			weARG_ERR_TX = 0x01, //Transmit error
			weARG_ERR_BU = 0x02, //Device busy
			weARG_ERR_RE = 0x03, //Device not ready
			weARG_ERR_PA = 0x04, //Wrong parameters
			weARG_ERR_NR = 0x05  //Device not reply
		};

		enum Addr {
			weBROADCAST = 0,		  //broadcast addr for rx-tx
			weBROADCAST_NO_ADDR = -1, //Use in Wake_Send to send as broadcast w/o addr-byte;
		};
		enum Crc {
			weCRC_INIT = 0x00, //by standart =  0xDE
			weCRC_POLY = 0x31, //x8+x5+x4+1
		};

	};

	struct TWakePacket {
		private:
			uint8_t* const _ptr;
		public:
			uint8_t* data() const {
				return _ptr + 2;
			}
			uint8_t& count() const {
				return _ptr[1];
			}
			uint8_t& cmd() const {
				return _ptr[0];
			}
			TWakePacket(const vd::gsl::span<uint8_t>& buf): _ptr(buf.data()) {}
	};


	struct TWakeAddress {
		private:
			//																				  V<--V
			//for internal use. Convert 16bit addr from (00xx xxxxx | xxxx xxxx) to  (0xxx xxxx | 0xxx xxxx)
			//#define WAKE_ADDRESS_SPLIT(x) (( (HI(x)&0x3f) <<9)|( (LO(x)<<1) &0x0100)|( LO(x)&0x7f ))
			constexpr static uint16_t _address_split(uint16_t x) {
				using namespace vd;
				return (((hiByte(x) & 0x3f) << 9) | ((lowByte(x) << 1) & 0x0100) | (lowByte(x) & 0x7f));
			}
			uint16_t _addr_converted;

		public:
			constexpr explicit TWakeAddress(uint16_t addr = TWakeInfo::weBROADCAST) : _addr_converted(
				    _address_split(addr)) {
				//assert(addr >= 0 && addr <= 0x3FFF); //cannot use bigger address
			}
			uint8_t getLowPart() const;

			uint8_t getHiPart() const;

			bool isLowPartSame(uint8_t lowAddr) const;

			bool isHiPartSame(uint8_t hiAddr) const;
	};



	namespace Private {
		struct TTranciever {
			private:
				bool _IsNeedStuff(u08* data) const;
			public:
				uint8_t _CRCValue;
				void reset();
				bool pushByte(uint8_t data, uint8_t** ptr);
				TTranciever();
				void addToCrc(u08 data, bool useCRC);
				void init();

		};

		struct TReciever {
			enum	TWakeState { WS_START = 0, WS_LOADDRESS, WS_HIADDRESS, WS_CMD, WS_COUNT, WS_DATA, WS_CRC, WS_COMPLETE };
			bool _wasSwapped;
			TWakeState		_wakeState;
			uint8_t _CRCValue;
			uint8_t _totalBytes;
			TWakeInfo::Error _errorCode;
			static bool isNewStart(u08 data);
			bool byteUnstuff(u08* data);
			void reset(TWakeInfo::Error errCode);
			void addToCrc(u08 data, bool useCRC);
			bool crcIsCorrect(u08 data);
			TReciever();
			TWakeInfo::Error getError();

		};


		class TWakeImpl {

				VOID_STATIC_ASSERT(TWakeInfo::FEND > 127); //send proc works only with this condition
				VOID_STATIC_ASSERT(TWakeInfo::FESC > 127); //send proc works only with this condition
			private:

				TWakeAddress _address;

				TReciever _rxData;
				TTranciever _txData;

				volatile u08 _packetCount;		//Count of received packets;
				//TODO
				TRingIterator<uint8_t> _it;
				uint8_t _recievedBytesInPacket;
				void _dropLastPacket(vd::gsl::span<uint8_t>& buf);
				void _commitPacket();
				void _saveByte(vd::gsl::span<uint8_t>& buf, uint8_t byte);
			public:
				enum TRxResult { RxDropByte, RxSave, RxReset };
				TWakeImpl();
				void setAddress(TWakeAddress selfAddr);

				bool onReceiveHandler(uint8_t Adata, vd::gsl::span<uint8_t>& buf);
				vd::gsl::span<uint8_t> makePacket(vd::gsl::span<uint8_t> out, s16 addr, u08 packet_cmd,
				                                  const vd::gsl::span<uint8_t>& data);
				bool getRecievedPacket(vd::gsl::span<uint8_t> out, vd::gsl::span<uint8_t> buf);

				void reset();

				//returns last error if happened(auto cleared after use).
				TWakeInfo::Error getError();

				uint8_t count() const;

		};

	}

	class TWakeProtocol {
		public:
		private:
			TWakeProtocol() {}
			typedef TWakeProtocol TSelf;
			typedef void(*onReceive_callback_t)(uint8_t cmd, uint8_t count, const uint8_t* data);



			static inline uint8_t* _rxBuffer = nullptr;
			static inline uint8_t _size = 0;

			static Private::TWakeImpl& _impl() {
				static Private::TWakeImpl _impl = Private::TWakeImpl();
				return _impl;
			}

		public:
			template <uint32_t SIZE>
			static void init(TWakeAddress addr) {
				static uint8_t buffer[SIZE] = {};
				VOID_STATIC_ASSERT(SIZE <= 255);
				_size = SIZE;
				_rxBuffer = &buffer[0];
				_impl().setAddress(addr);
				reset();
			}
			static void deinit() {
			}
			static void reset() {
				ATOMIC_BLOCK(vd::ON_EXIT::RESTORE_STATE) {
					_impl().reset(); // reset  FSM of wake
				}
			}
			static void handleData(const uint8_t& data) {
				_impl().onReceiveHandler(data, vd::gsl::span<uint8_t>(_rxBuffer, _size));
			}
			static void handleData(uint8_t* data, uint32_t len) {
				uint8_t* ptr = data;

				for (uint32_t i = 0; i < len; ++i, ++ptr) {
					handleData(*ptr);
				}
			}


			static bool readPacketAsync(vd::gsl::span<uint8_t> out) {
				return _impl().getRecievedPacket(out, vd::gsl::span<uint8_t>(_rxBuffer, _size));
			}
			//if addr=-1 then send without address=BROADCAST. By default addr=0 also BROADCAST but it still sended(as 0x80);
			//return length

			static vd::gsl::span<uint8_t> makePacket(vd::gsl::span<uint8_t> buf, s16 addr, uint8_t cmd,
			        const vd::gsl::span<uint8_t>& data = vd::gsl::span<uint8_t>()) {
				return _impl().makePacket(buf, addr, cmd, data);
			}

			//Get rx packet count
			static uint8_t count() {
				return _impl().count();
			}
			//returns last error if happened(auto cleared after use).
			static TWakeInfo::Error getError() {
				TWakeInfo::Error err;
				ATOMIC_BLOCK(vd::ON_EXIT::RESTORE_STATE) {
					err = _impl().getError();
				}
				return err;
			}
	};
	/*
	template <uint8_t INT_BUFFER_SIZE>
	Private::TWakeImpl TWakeProtocol<INT_BUFFER_SIZE>::_impl =
	    Private::TWakeImpl();

	template <uint8_t INT_BUFFER_SIZE>
	uint8_t cph::TWakeProtocol<INT_BUFFER_SIZE>::_rxBuffer[INT_BUFFER_SIZE];
	*/



}
