#pragma once

enum MBDirection { DIRECTION_RX, DIRECTION_TX };


/* ----------------------- Serial port functions ----------------------------*/
class ModbusRTU;
struct TransportAdapter {
	virtual bool init() = 0;
	virtual bool writeByte(uint8_t byte) = 0;
	virtual void setDirection(MBDirection) = 0;
	virtual uint32_t getBaudrate() = 0;
};

/* ----------------------- Timers functions ---------------------------------*/

struct TimerAdapter {
	virtual bool	init(uint32_t usTickTime) = 0;
	virtual void	close(void) = 0;
	virtual void	enable(void) = 0;
	virtual void	reset() = 0;
	virtual void	disable(void) = 0;
	virtual void	delay(uint16_t usTimeOutMS) = 0;
};

