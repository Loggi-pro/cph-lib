<?xml version="1.0" encoding="utf-8"?>
<d:error xmlns:d="DAV:" xmlns:s="http://sabredav.org/ns">
  <s:exception>Doctrine\DBAL\Exception\UniqueConstraintViolationException</s:exception>
  <s:message>An exception occurred while executing 'INSERT INTO `oc_file_locks` (`key`,`lock`,`ttl`) SELECT ?,?,? FROM `oc_file_locks` WHERE `key` = ? HAVING COUNT(*) = 0' with params ["files\/30762b9c033685010b0128ff3b0910f5", 1, 1586862132, "files\/30762b9c033685010b0128ff3b0910f5"]:

SQLSTATE[23000]: Integrity constraint violation: 1062 Duplicate entry 'files/30762b9c033685010b0128ff3b0910f5' for key 'lock_key_index'</s:message>
</d:error>
te);
			}
		public:


			template <uint32_t BAUDRATE,
			          typename Flags::UsartDataBits DB = Flags::DataBits8,
			          typename  Flags::UsartStopBits SB = Flags::OneStopBit,
			          typename  Flags::UsartParity P = Flags::NoneParity,
			          typename  Flags::UsartMode M = Flags::RxTxEnable
			          >
			static void init() {
				Hardware::template init<BAUDRATE, DB, SB, P, M>();
				_buffer.clear();
				Hardware::setCallback(nullptr, &_cbTranceive);
			}

			static void init(uint32_t baudrate,
			                 typename  Flags::UsartDataBits db = Flags::DataBits8,
			                 typename Flags::UsartStopBits sb = Flags::OneStopBit,
			                 typename Flags::UsartParity p = Flags::NoneParity,
			                 typename Flags::UsartMode m = Flags::RxTxEnable) {
				Hardware::init(baudrate, db, sb, p, m);
				_buffer.clear();
				Hardware::setCallback(nullptr, &_cbTranceive);
			}
			static void setMode(typename Flags::UsartMode m) {
				Hardware::setMode(m);
			}
			static typename Flags::UsartMode getMode() {
				return Hardware::getMode();
			}


			static void deinit() {
				Hardware::deinit();
			}

			static bool writeData(const vd::gsl::span<uint8_t>& data) {
				if (data.size() > (uint32_t)(_buffer.capacity() - _buffer.count())) { return false; }

				for (size_t i = 0; i < data.size(); i++) {
					uint8_t byte = data.data()[i];
					_buffer.push(byte);
				}

				Hardware::StartTranceive();
				return true;
			}
			static bool writeByte(uint8_t byte) {
				bool res = _buffer.push(byte);

				if (res) {
					Hardware::StartTranceive();
				}

				return res;
			}
			static bool isSended() {
				return _buffer.is_empty();
			}

			static void reset() {
				_buffer.clear();
			}

			static void setOnReceive(typename Hardware::cbOnRecieve_t onDataReceiveCallback) {
				Hardware::setCallback(onDataReceiveCallback, &_cbTranceive);
			}

			static typename Flags::Error getError() {
				return Hardware::getError();
			}
	};

}