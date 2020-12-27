#include "wake.h"
#include "crc.h"
//template vd:: Calc_crc8<cph::TWakeInfo::weCRC_POLY>(u08* varCrc, u08 data);
namespace cph {

	static const TWakeAddress F_BROADCAST = TWakeAddress(TWakeInfo::weBROADCAST);


	//constexpr uint16_t TWakeAddress::_address_split(uint16_t x)
	//constexpr TWakeAddress::TWakeAddress(uint16_t addr)

	uint8_t TWakeAddress::getLowPart() const {
		return vd::lowByte(_addr_converted);
	}
	uint8_t TWakeAddress::getHiPart() const {
		return vd::hiByte(_addr_converted);
	}
	bool TWakeAddress::isLowPartSame(uint8_t lowAddr) const {
		return lowAddr == getLowPart() ||
		       lowAddr == F_BROADCAST.getLowPart();
	}
	bool TWakeAddress::isHiPartSame(uint8_t hiAddr) const {
		return hiAddr == getHiPart() ||
		       hiAddr == F_BROADCAST.getHiPart();
	}



	namespace Private {
		//================================RECIEVER=======================
		Private::TReciever::TReciever() {
			reset(TWakeInfo::weNO_ERROR);
		}

		TWakeInfo::Error TReciever::getError() {
			TWakeInfo::Error result;
			result = _errorCode;
			_errorCode = TWakeInfo::weNO_ERROR;
			return result;
		}

		bool TReciever::isNewStart(u08 data) {
			return  (data == TWakeInfo::FEND);
		}

		bool TReciever::byteUnstuff(u08* data) {
			bool result = true;

			if (*data == TWakeInfo::FESC) {
				_wasSwapped = true;
				return false;
			};

			if (_wasSwapped) {
				//Wake_status.WasSwapped = 1;
				_wasSwapped = false;

				if (*data == TWakeInfo::TFEND) { *data = TWakeInfo::FEND; }
				else if (*data == TWakeInfo::TFESC) { *data = TWakeInfo::FESC; }
			}

			return result;
		}



		void TReciever::reset(TWakeInfo::Error errCode) {
			_wasSwapped = false;
			_wakeState = WS_START;
			_CRCValue = TWakeInfo::weCRC_INIT;
			_totalBytes = 0;
			_errorCode = errCode;
		}

		bool TReciever::crcIsCorrect(u08 data) {
			return _CRCValue == data;
		}
		void TReciever::addToCrc(u08 data, bool useCRC) {
			if (!useCRC) { return; }

			//Calc_crc8<TWakeInfo::weCRC_POLY>(&_CRCValue, data);
			Calc_crc8(&_CRCValue, TWakeInfo::weCRC_POLY, data);
		}

		//==================================================================
		//==========================TRANCIEVER=============================
		TTranciever::TTranciever() {
			reset();
		}
		bool TTranciever::_IsNeedStuff(u08* data)const {
			bool result = false;

			if (*data == TWakeInfo::FEND) {
				*data = TWakeInfo::TFEND;
				result = true;
			} else if (*data == TWakeInfo::FESC) {
				*data = TWakeInfo::TFESC;
				result = true;
			}

			return result;
		}

		void TTranciever::reset() {
			_CRCValue = TWakeInfo::weCRC_INIT;
		}

		void TTranciever::addToCrc(u08 data, bool useCRC) {
			if (!useCRC) { return; }

			//Calc_crc8<TWakeInfo::weCRC_POLY>(&_CRCValue, data);
			Calc_crc8(&_CRCValue, TWakeInfo::weCRC_POLY, data);
		}
		bool TTranciever::pushByte(uint8_t data, uint8_t** ptr) {
			uint8_t _currentByte = data;

			if (_IsNeedStuff(&_currentByte)) {
				** ptr = TWakeInfo::FESC;
				*ptr = *ptr + 1;
			}

			** ptr = _currentByte;
			*ptr = *ptr + 1;
			return true;
		}

		void TTranciever::init() {
			_CRCValue = TWakeInfo::weCRC_INIT;
		}

		//=================================================================


		TWakeImpl::TWakeImpl() : _address(), _rxData(), _txData(), _packetCount(0), _it() {
		}

		void TWakeImpl::setAddress(TWakeAddress selfAddr) {
			_address = selfAddr;
		}

		void TWakeImpl::_dropLastPacket(vd::gsl::span<uint8_t>& buf) {
			while (_recievedBytesInPacket > 0) {
				_it.pop_back(buf);
				_recievedBytesInPacket--;
			}
		}

		void TWakeImpl::_commitPacket() {
			_recievedBytesInPacket = 0;
		}

		void TWakeImpl::_saveByte(vd::gsl::span<uint8_t>& buf, uint8_t byte) {
			if (_it.push(buf, byte)) {
				_recievedBytesInPacket++;
			}
		}

		bool TWakeImpl::onReceiveHandler(uint8_t Adata, vd::gsl::span<uint8_t>& buf) {
			bool repeat = false;
			bool result = false;
			const bool useCRC = true;

			if (_rxData._wakeState != TReciever::WS_START) {
				if (_rxData.isNewStart(Adata)) {
					if (_rxData._wakeState == TReciever::WS_COMPLETE) {
						_rxData.reset(TWakeInfo::weNO_ERROR);
					} else {
						_rxData.reset(TWakeInfo::weFORMAT);
					}
				}
			}

			if (!_rxData.byteUnstuff(&Adata)) { return false; }

			//KA
			do {
				repeat = false;

				switch (_rxData._wakeState) {
				case TReciever::WS_START:
					if (!TReciever::isNewStart(Adata)) { break; }

					_rxData.reset(TWakeInfo::weNO_ERROR);
					_rxData.addToCrc(Adata, useCRC);
					_rxData._wakeState = TReciever::WS_LOADDRESS;
					_dropLastPacket(buf);
					break;

				case TReciever::WS_LOADDRESS:

					//res = false;
					if (Adata >= 128) {
						_rxData._wakeState = TReciever::WS_HIADDRESS;
						Adata &= (~0x80);

						if (_address.isLowPartSame(Adata)) {
							_rxData.addToCrc(Adata, useCRC);
						} else {
							_rxData.reset(TWakeInfo::weADDR);
							_dropLastPacket(buf);
						}

						break;
					} else { //data<128
						_rxData._wakeState = TReciever::WS_CMD;
						repeat = true;
					}

					break;

				case TReciever::WS_HIADDRESS:
					if (Adata >= 128) {
						_rxData._wakeState = TReciever::WS_CMD;
						Adata &= (~0x80);

						if (_address.isHiPartSame(Adata)) {
							_rxData.addToCrc(Adata, useCRC);
						} else {
							_dropLastPacket(buf);
							_rxData.reset(TWakeInfo::weADDR);
						}

						break;
					} else { //data<128
						_rxData._wakeState = TReciever::WS_CMD;
						repeat = true;
					}

					break;

				case TReciever::WS_CMD:
					if (Adata < 128) {
						_rxData.addToCrc(Adata, useCRC);
						_rxData._wakeState = TReciever::WS_COUNT;
						_saveByte(buf, Adata);
					} else {
						_rxData.reset(TWakeInfo::weFORMAT);
						_dropLastPacket(buf);
					}

					break;

				case TReciever::WS_COUNT:
					_rxData.addToCrc(Adata, useCRC);
					_rxData._totalBytes = Adata;
					_rxData._wakeState = TReciever::WS_DATA;
					_saveByte(buf, Adata);
					break;

				case TReciever::WS_DATA:
					if (_rxData._totalBytes > 0) {
						_rxData.addToCrc(Adata, useCRC);
						_rxData._totalBytes--;
						_saveByte(buf, Adata);

						if (_rxData._totalBytes == 0) {
							if (useCRC == true) {
								_rxData._wakeState = TReciever::WS_CRC;
							} else {
								_rxData._wakeState = TReciever::WS_COMPLETE;
								repeat = true;
							}
						}
					} else {
						repeat = true;

						if (useCRC == true) {
							_rxData._wakeState = TReciever::WS_CRC;
						} else {
							_rxData._wakeState = TReciever::WS_COMPLETE;
						}
					}

					break;

				case TReciever::WS_CRC:
					if (!_rxData.crcIsCorrect(Adata)) {
						_rxData.reset(TWakeInfo::weCRC);
						_dropLastPacket(buf);
						break;
					}

				case TReciever::WS_COMPLETE:
					_rxData.reset(TWakeInfo::weNO_ERROR);
					result = true;
					_commitPacket();
					vd::AtomicFetchAndAdd(&_packetCount, 1);
					break;

				default:
					_rxData.reset(TWakeInfo::weFORMAT);
					_dropLastPacket(buf);
					break;
				};
			} while (repeat);

			return result;
		}
		/*	bool ret_result = false;
			uint8_t* ptr = out.data();
			switch (_txData._sendState) {
			case TTranciever::WSS_START:
				_txData.init();
				*ptr = TWakeInfo::FEND;
				if (writeData(TWakeInfo::FEND)) {
					_txData.addToCrc(TWakeInfo::FEND, _USE_CRC);
					_txData._sendState = TTranciever::WSS_LOADDRESS;
				}

				break;

			case TTranciever::WSS_LOADDRESS:
				if (addr >= 0) {
					const TWakeAddress addrConverted(addr);
					uint8_t lowAddress = addrConverted.getLowPart();

					if (_txData.sendByte(lowAddress | 0x80, writeData)) {
						_txData.addToCrc(lowAddress, _USE_CRC);

						if (addr > 127) {
							_txData._sendState = TTranciever::WSS_HIADDRESS;
						} else {
							_txData._sendState = TTranciever::WSS_CMD;
						}
					}
				} else {
					_txData._sendState = TTranciever::WSS_CMD;
				}

				break;

			case TTranciever::WSS_HIADDRESS: {
				const TWakeAddress addrConverted(addr);
				uint8_t hiAddress = addrConverted.getHiPart();

				if (_txData.sendByte(hiAddress | 0x80, writeData)) {
					_txData.addToCrc(hiAddress, _USE_CRC);
					_txData._sendState = TTranciever::WSS_CMD;
				}

				break;
			}

			case TTranciever::WSS_CMD: {
				uint8_t data = packet_cmd & 0x7f;

				if (_txData.sendByte(data, writeData)) {
					_txData.addToCrc(data, _USE_CRC);
					_txData._sendState = TTranciever::WSS_COUNT;
				}

				break;
			}

			case TTranciever::WSS_COUNT:
				_txData._totalBytes = 0;

				if (_txData.sendByte(packet_count, writeData)) {
					_txData.addToCrc(packet_count, _USE_CRC);
					_txData._sendState = TTranciever::WSS_DATA;
				}

				break;

			case TTranciever::WSS_DATA:
				if (_txData._totalBytes < packet_count) {
					uint8_t data = packet_data[_txData._totalBytes];

					if (_txData.sendByte(data, writeData)) {
						_txData.addToCrc(data, _USE_CRC);
						_txData._totalBytes++;

						if (_txData._totalBytes == packet_count) {
							_txData._sendState = TTranciever::WSS_CRC;
						};
					}
				} else {
					_txData._sendState = TTranciever::WSS_CRC;
				}

				break;

			case TTranciever::WSS_CRC:
				if (!_USE_CRC) { _txData._sendState = TTranciever::WSS_COMPLETE; }

				if (_txData.sendByte(_txData._CRCValue, writeData)) {
					_txData._sendState = TTranciever::WSS_COMPLETE;
				}

				break;

			case TTranciever::WSS_COMPLETE:
				_txData._sendState = TTranciever::WSS_START;
				ret_result = true;
			}

			return ret_result;*/
		vd::gsl::span<uint8_t> TWakeImpl::makePacket(vd::gsl::span<uint8_t> out, s16 addr, u08 packet_cmd,
		        const vd::gsl::span<uint8_t>& data) {
			uint8_t* ptr = out.data();
			bool flagSendHiAddress = false;
			const bool useCRC = true;
			//start byte
			_txData.init();
			*ptr++ = TWakeInfo::FEND;
			_txData.addToCrc(TWakeInfo::FEND, useCRC);

			//ADDRESS:LOW

			if (addr >= 0) {
				flagSendHiAddress = addr > 127;
				const TWakeAddress addrConverted(addr);
				uint8_t lowAddress = addrConverted.getLowPart();

				if (!_txData.pushByte(lowAddress | 0x80, &ptr)) { return out.subspan(0, 0); }

				_txData.addToCrc(lowAddress, useCRC);
			}

			//ADDRESS:HIGH
			if (flagSendHiAddress) {
				const TWakeAddress addrConverted(addr);
				uint8_t hiAddress = addrConverted.getHiPart();

				if (!_txData.pushByte(hiAddress | 0x80, &ptr)) { return out.subspan(0, 0); }

				_txData.addToCrc(hiAddress, useCRC);
			}

			//cmd
			uint8_t cmd = packet_cmd & 0x7f;

			if (!_txData.pushByte(cmd, &ptr))  { return out.subspan(0, 0); }

			_txData.addToCrc(cmd, useCRC);
			//count
			uint8_t totalBytes = 0;

			if (!_txData.pushByte(data.size(), &ptr)) { return out.subspan(0, 0); }

			_txData.addToCrc(data.size(), useCRC);

			//data
			while (totalBytes < data.size()) {
				uint8_t byte = data[totalBytes];

				if (!_txData.pushByte(byte, &ptr)) { return out.subspan(0, 0); }

				_txData.addToCrc(byte, useCRC);
				totalBytes++;

				if (totalBytes == data.size()) {
					break;
				};
			}

			//CRC
			if (!useCRC) { return ptr - out.data();}

			if (!_txData.pushByte(_txData._CRCValue, &ptr)) { return out.subspan(0, 0); }

			return out.subspan(0, ptr - out.data());
		}

		bool TWakeImpl::getRecievedPacket(vd::gsl::span<uint8_t> out, vd::gsl::span<uint8_t> buf) {
			if (!_packetCount) { return false; }

			vd::Atomic::FetchAndSub(&_packetCount, 1);
			uint8_t* ptr = out.data();
			uint8_t* cmd = ptr++;
			_it.pop(buf, cmd);
			uint8_t* len = ptr++;
			_it.pop(buf, len);
			uint8_t size = *len;

			if (size > out.size()) {
				while (size > 0) {
					if (!_it.pop(buf, ptr)) { break; }
				}

				assert(false);
				return false;
			}

			while (size > 0) {
				size--;

				if (!_it.pop(buf, ptr++)) {
					assert(false);
					return false;
				}
			}

			return true;
		}
		void TWakeImpl::reset() {
			_rxData.reset(TWakeInfo::weNO_ERROR);
			_packetCount = 0;
			_recievedBytesInPacket = 0;
			_it.clear();
		}
		TWakeInfo::Error TWakeImpl::getError() {
			TWakeInfo::Error err;
			err = _rxData.getError();
			return err;
		}
		uint8_t TWakeImpl::count()const {
			uint8_t count =  vd::Atomic::Fetch(&_packetCount);
			return count;
		}

	}
}
