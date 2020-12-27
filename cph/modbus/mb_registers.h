#pragma once
#include "mbtypes.h"
template <uint8_t id, uint16_t INPUT_REGS, uint16_t HOLDING_REGS, uint16_t COILS_REGS, uint16_t DISCRETE_REGS>
struct ModbusRegs {
	static inline uint16_t	usRegInputBuf[INPUT_REGS] = { 0 };
	static inline uint16_t	usRegHoldingBuf[HOLDING_REGS] = { 0 };
	static inline bool	usRegCoilsBuf[COILS_REGS] = { 0 };
	static inline bool	usRegDiscreteBuf[DISCRETE_REGS] = { 0 };
	static uint16_t changeEndianness(uint16_t reg) {
		uint8_t lowPart = (reg & 0xFF);
		uint8_t hiPart = (reg >> 8);
		uint16_t res = (lowPart << 8) | hiPart;
		return res;
	}

	static bool isAddressCorrect(uint16_t intitialAddress, uint16_t numOfCoils,
	                             uint16_t countOfRegisters) {
		if (numOfCoils == 0) { return false; }

		return ((intitialAddress + numOfCoils) <= countOfRegisters);
	}

	static MBErrorCode readInputRegister(uint16_t* data, uint16_t address, uint16_t count) {
		if (!isAddressCorrect(address, count, INPUT_REGS)) { return MB_ENOREG; }

		for (uint16_t i = 0; i < count; i++, address++, data++) {
			uint16_t index = address + i;
			(*data) = changeEndianness(usRegInputBuf[index]);
		}

		return MB_ENOERR;
	}
	static MBErrorCode readHoldingRegister(uint16_t* data, uint16_t address, uint16_t count) {
		if (!isAddressCorrect(address, count, HOLDING_REGS)) { return MB_ENOREG; }

		for (uint16_t i = address; i < address + count; i++, data++) {
			(*data) = changeEndianness(usRegHoldingBuf[i]);
		}

		return MB_ENOERR;
	}

	static MBErrorCode writeHoldingRegister(const uint16_t* data, uint16_t address, uint16_t count) {
		if (!isAddressCorrect(address, count, HOLDING_REGS)) { return MB_ENOREG; }

		for (uint16_t i = address; i < address + count; i++, data++) {
			usRegHoldingBuf[i] = changeEndianness(*data);
		}

		return MB_ENOERR;
	}
	static MBErrorCode readCoil(bit_array_t& bits, uint16_t address, uint16_t count) {
		if (!isAddressCorrect(address, count, COILS_REGS)) { return MB_ENOREG; }

		for (uint16_t i = 0; i < count; i++, address++) {
			bits[i] = usRegCoilsBuf[address];
		}

		return MB_ENOERR;
	}

	static MBErrorCode writeCoil(const bit_array_t& bits, uint16_t address, uint16_t count) {
		if (!isAddressCorrect(address, count, COILS_REGS)) { return MB_ENOREG; }

		for (uint16_t i = 0; i < count; i++, address++) {
			usRegCoilsBuf[address] = bits[i];
		}

		return MB_ENOERR;
	}

	static MBErrorCode readDiscrete(bit_array_t& bits, uint16_t address, uint16_t count) {
		if (!isAddressCorrect(address, count, DISCRETE_REGS)) { return MB_ENOREG; }

		for (uint16_t i = 0; i < count; i++, address++) {
			bits[i] = usRegDiscreteBuf[address];
		}

		return MB_ENOERR;
	}

};

