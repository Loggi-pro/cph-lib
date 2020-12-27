#include <cph/modbus/mb.h>
#include <cph/modbus/mbrtu.h>
#include <cph/modbus/mbport.h>
#include <cph/timer.h>
#include <cph/basic_usart.h>
#include <void/interrupt.h>
#define MODBUS_ADDR 0x04
#define MODBUS_ADDR_TEMPERATURE 0x01
#define MODBUS_ADDR_PINS 0x04
#define MODBUS_ADDR_RELAY_STATE 0x0B
#define MODBUS_ADDR_RESET 0x0C
#define MODBUS_ADDR_RELAY_WRITE 0x21


u08 my_var = 12;
bool flag = true;
MBErrorCode readHoldingReg(uint16_t* buff, uint16_t address, uint16_t count) {
	MBErrorCode err = MB_ENOERR;

	switch (address) {
	case MODBUS_ADDR_TEMPERATURE:
		(*buff) = 2075;
		break;

	case MODBUS_ADDR_PINS:
		(*buff) = my_var;
		break;

	case MODBUS_ADDR_RELAY_STATE:
		(*buff) = flag ? 1 : 0;
		break;

	default:
		return MB_ENOREG;
	}

	return err;
}

MBErrorCode writeHoldingReg(const uint16_t* buff, uint16_t address, uint16_t count) {
	MBErrorCode err = MB_ENOERR;

	switch (address) {
	case MODBUS_ADDR_RELAY_WRITE:
		flag = (*buff) == 1 ? true : false;
		break;

	case MODBUS_ADDR_RESET:
		break;

	default:
		return MB_ENOREG;
	}

	return err;
}


#include <cph/iostream.h>
#include <cph/pwm.h>
#include <cph/gpio.h>
//typedef cph::basic_ostream<cph::AdapterUsartSync_Debug<cph::Usart0>> ostream;
//ostream cout;
typedef cph::UsartAsync<cph::Usart0, 255> TUsart;

typedef cph::io::Pd4 pinUartTxEn;

extern "C" {
	VOID_ISR(CPH_INT_USART0_RXC) {
		TUsart::Hardware::IntRxHandler();
	}

	VOID_ISR(CPH_INT_USART0_TXE) {
		TUsart::Hardware::IntTxEmptyHandler();
	}
}

Modbus mb;




struct TransportAdapterImpl : TransportAdapter {
	static inline const uint32_t BAUDRATE = 19200;
	static void onReceive(const uint8_t& b) {
		mb.instance().handleReceived(b);
	}
	virtual bool init() override {
		TUsart::init<BAUDRATE>();
		TUsart::setOnReceive(&onReceive);
		return true;
	}
	virtual  bool writeByte(u08 byte) override {
		return TUsart::writeByte(byte);
	}
	virtual void setDirection(MBDirection dir) override {
		switch (dir) {
		case DIRECTION_RX:
			pinUartTxEn::Clear();
			break;

		case DIRECTION_TX:
			pinUartTxEn::Set();
			break;

		default:
			pinUartTxEn::Clear();//enable�rx�by�default
			break;
		}
	}
	virtual uint32_t getBaudrate() override {
		return BAUDRATE;
	}
};

struct TimerAdapterImpl : TimerAdapter {
	static inline const cph::TTimerNumber TIMER_N = cph::TIMER_1;
	static void TimerExpired() {
		mb.instance().handleTimerT35Expired();
	}
	virtual bool init(uint32_t usTickTime) override {
		//usTimeTick *= 2;
		vd::chrono::micro_t t(usTickTime);
		cph::TCounter::initCustom<TIMER_N>(t, &TimerExpired);
		//cph::TCounter::initCustom<cph::TIMER_1>(2_ms, &TimerExpired);
		cph::TCounter::disable<TIMER_N>();
		return true;
		//TODO call onExpire
	}
	virtual void close() override {
	}
	virtual void enable() override {
		cph::TCounter::enable<TIMER_N>();
	}
	virtual void reset() override {
		cph::TCounter::reset<TIMER_N>();
	}
	virtual void disable() override {
		cph::TCounter::disable<TIMER_N>();
	}
	virtual void delay(u16 usTimeOutMS)override {
		return;
	}
};


TimerAdapterImpl timerAdapter;

TransportAdapterImpl transport;







int main() {
	pinUartTxEn::Clear(); //TODO debug only (allow rx)
	pinUartTxEn::SetConfiguration(pinUartTxEn::Port::Out);
	TUsart::setOnReceive(&transport.onReceive);
	cph::TSystemCounter::initPrecision<cph::TimerPrecision::Resolution_1ms, cph::TTimerNumber::TIMER_0>();
	cph::TSystemCounter::enable < cph::TTimerNumber::TIMER_0>();
	mb.init(&transport, &timerAdapter, 0x04);
	mb.setRegisterHoldingCallback(readHoldingReg, writeHoldingReg);
	vd::EnableInterrupts();
	mb.enable();
	cph::timer_t t;

	while (true) {
		mb.poll();
	}

	return 0;
}