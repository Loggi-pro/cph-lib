#pragma once
#include "mbtypes.h"
#include "mbframe.h"
struct TransportAdapter;
struct TimerAdapter;
class ModbusEventQueue;
class ModbusRTU {
	private:
		friend class Modbus;

		enum eMBRcvState {
			STATE_RX_INIT,              /*!< Receiver is in initial state. */
			STATE_RX_IDLE,              /*!< Receiver is in idle state. */
			STATE_RX_RCV,               /*!< Frame is beeing received. */
			STATE_RX_ERROR              /*!< If the frame is invalid. */
		};

		enum eMBSndState {
			STATE_TX_IDLE,              /*!< Transmitter is in idle state. */
			STATE_TX_XMIT               /*!< Transmitter is in transfer state. */
		};
		TransportAdapter* _adapter;
		TimerAdapter* _timer;
		ModbusEventQueue* _queue;
		volatile eMBSndState _stateSend;
		volatile eMBRcvState _stateReceive;
		volatile uint8_t  _bufferRTU[MB_SER_PDU_SIZE_MAX];
		volatile uint16_t _bufferReceivePos;

		ModbusRTU() = default;
		MBErrorCode		init(TransportAdapter* adapter, TimerAdapter* timer, ModbusEventQueue* queue);
		void            start();
		void            stop();
		void			close();
		MBErrorCode		receive(uint8_t* rxAddress, uint8_t** frame, uint16_t* length);
		MBErrorCode		send(uint8_t slaveAddress, const uint8_t* frame, uint16_t length);
	public:
		void            handleReceived(const uint8_t& val);
		bool            handleTimerT35Expired();
};

