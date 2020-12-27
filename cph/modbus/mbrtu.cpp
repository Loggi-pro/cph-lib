#include <void/assert.h>
#include <void/interrupt.h>
#include <void/atomic.h>

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbrtu.h"

#include "mbutils.h"
#include "mbport.h"

/* ----------------------- Start implementation -----------------------------*/

MBErrorCode ModbusRTU::init(TransportAdapter* adapter, TimerAdapter* timer,
                            ModbusEventQueue* queue) {
	this->_queue = queue;
	this->_adapter = adapter;
	this->_timer = timer;
	ATOMIC_BLOCK(vd::ON_EXIT::RESTORE_STATE) {
		/* Modbus RTU uses 8 Databits. */
		if (!adapter->init()) {
			return MB_EPORTERR;
		}

		/* If baudrate > 19200 then we should use the fixed timer values
		 * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
		 */
		u32 baudrate = adapter->getBaudrate();
		const 	u32 usTimerDelay = baudrate > 19200 ? 1800 : (7UL * 11UL * 1000000UL) /
		                           (2UL * baudrate);

		/* The timer reload value for a character is given by:
		 *
		 * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
		 *             = 11 * Ticks_per_1s / Baudrate
		 *             = 220000 / Baudrate
		 * The reload for t3.5 is 1.5 times this value and similary
		 * for t3.5.
		 */

		if (!_timer->init(usTimerDelay)) {
			return MB_EPORTERR;
		}
	}
	return MB_ENOERR;
}

void ModbusRTU::start() {
		/* Initially the receiver is in the state STATE_RX_INIT. we start
		 * the timer and if no character is received within t3.5 we change
		 * to STATE_RX_IDLE. This makes sure that we delay startup of the
		 * modbus protocol stack until the bus is free.
		 */
		_stateReceive = ModbusRTU::STATE_RX_INIT;
		_stateSend = ModbusRTU::STATE_TX_IDLE; //init of _eSndState;
		_adapter->setDirection(DIRECTION_RX);
		_timer->enable();
}

void ModbusRTU::stop() {
		_adapter->setDirection(DIRECTION_TX);
		_timer->disable();
}

void ModbusRTU::close() {}

MBErrorCode ModbusRTU::receive( uint8_t* rxAddress, uint8_t** pucFrame, uint16_t* length ) {
		assert(_bufferReceivePos < MB_SER_PDU_SIZE_MAX);

		/* Length and CRC check */
		if (!((_bufferReceivePos >= MB_SER_PDU_SIZE_MIN)
		        && (calc_crc16((uint8_t*)_bufferRTU, _bufferReceivePos) == 0))) {
			return MB_EIO;
		}

		/* Save the address field. All frames are passed to the upper layed
		 * and the decision if a frame is used is done there.
		 */
		*rxAddress = _bufferRTU[MB_SER_PDU_ADDR_OFF];
		/* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
		 * size of address field and CRC checksum.
		 */
		*length = (uint16_t)(_bufferReceivePos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC);
		/* Return the start of the Modbus PDU to the caller. */
		*pucFrame = (uint8_t*)& _bufferRTU[MB_SER_PDU_PDU_OFF];
	return MB_ENOERR;
}

MBErrorCode ModbusRTU::send( uint8_t slaveAddress, const uint8_t* frame, uint16_t length ) {
	MBErrorCode result = MB_ENOERR;
	/* Check if the receiver is still in idle state. If not we where to
	 * slow with processing the received frame and the master sent another
	 * frame on the network. We have to abort sending the frame.
	 */
		if (_stateReceive != STATE_RX_IDLE) {return MB_EIO;}

		/* First byte before the Modbus-PDU is the slave address. */
		uint8_t* bufferPtr = (uint8_t*)frame - 1;
		uint16_t len = 1;
		/* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
		bufferPtr[MB_SER_PDU_ADDR_OFF] = slaveAddress;
		len += length;
		/* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
		uint16_t usCRC16 = calc_crc16((uint8_t*)bufferPtr, len);
		_bufferRTU[len++] = (uint8_t)(usCRC16 & 0xFF);
		_bufferRTU[len++] = (uint8_t)(usCRC16 >> 8);
		/* Activate the transmitter. */
		_stateSend = STATE_TX_XMIT;
		_adapter->setDirection(DIRECTION_TX);
		//send data

		while (len) {
			if (!_adapter->writeByte(*bufferPtr)) {
				result = MB_EIO;
				break;
			}

			bufferPtr++;  /* next byte in sendbuffer. */
			--len;
		}

		if (result == MB_ENOERR) {_queue->postEvent(ModbusEventQueue::EV_FRAME_SENT);}

		_stateSend = STATE_TX_IDLE;
		_adapter->setDirection(DIRECTION_RX);
	return result;
}
//#include "hardware.h"
void ModbusRTU::handleReceived( const uint8_t& val) {
	uint8_t           ucByte = val;
	/* Always read the character. */
	assert(_stateSend == STATE_TX_IDLE);
	//dprintf(PSTR("Rx interrupt:"));
	//cout << "get=" << val << cph::endl;

	switch ( _stateReceive ) {
	/* If we have received a character in the init state we have to
	 * wait until the frame is finished.
	 */
	case STATE_RX_INIT:
		_timer->enable(  );
		//dprintf(PSTR("wait init:%X\r\n"), ucByte);
		break;

	/* In the error state we wait until all characters in the
	 * damaged frame are transmitted.
	 */
	case STATE_RX_ERROR:
		_timer->enable(  );
		//dprintf(PSTR("error:%X\r\n"), ucByte);
		break;

	/* In the idle state we wait for a new character. If a character
	 * is received the t1.5 and t3.5 timers are started and the
	 * receiver is in the state STATE_RX_RECEIVCE.
	 */
	case STATE_RX_IDLE:
		_bufferReceivePos = 0;
		_bufferRTU[_bufferReceivePos++] = ucByte;
		_stateReceive = STATE_RX_RCV;
		/* Enable t3.5 timers. */
		//dprintf(PSTR("rx byte[RX_IDLE]:%X\r\n"), ucByte);
		_timer->enable();
		break;

	/* We are currently receiving a frame. Reset the timer after
	 * every character received. If more than the maximum possible
	 * number of bytes in a modbus frame is received the frame is
	 * ignored.
	 */
	case STATE_RX_RCV:
		if ( _bufferReceivePos < MB_SER_PDU_SIZE_MAX ) {
			_bufferRTU[_bufferReceivePos++] = ucByte;
			//dprintf(PSTR("rx byte[RX_RCV]:%X\r\n"), ucByte);
		} else {
			_stateReceive = STATE_RX_ERROR;
		}

		_timer->reset();
		break;
	}

	return;
}

bool ModbusRTU::handleTimerT35Expired() {
	bool            xNeedPoll = false;
	//cout << "Timer expired\r\n";

	switch ( _stateReceive ) {
	/* Timer t35 expired. Startup phase is finished. */
	case STATE_RX_INIT:
		xNeedPoll = _queue->postEvent(ModbusEventQueue::EV_READY);
		break;

	/* A frame was received and t35 expired. Notify the listener that
	 * a new frame was received. */
	case STATE_RX_RCV:
		//cout << "Handle frame\r\n";
		xNeedPoll = _queue->postEvent(ModbusEventQueue::EV_FRAME_RECEIVED);
		break;

	/* An error occured while receiving the frame. */
	case STATE_RX_ERROR:
		break;

	/* Function called in an illegal state. */
	default:
		assert( ( _stateReceive == STATE_RX_INIT ) ||
		        ( _stateReceive == STATE_RX_RCV ) || ( _stateReceive == STATE_RX_ERROR ) );
	}

	_timer->disable();
	_stateReceive = STATE_RX_IDLE;
	return xNeedPoll;
}
