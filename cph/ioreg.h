#pragma once

#define IO_REG_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE) \
	struct CLASS_NAME\
	{\
		typedef DATA_TYPE DataT;\
		static DataT Get(){return REG_NAME;}\
		static void Set(DataT value){REG_NAME = (DataT)(value);}\
		static void Or(DataT value){REG_NAME |= (DataT)(value);}\
		static void And(DataT value){REG_NAME &= (DataT)(value);}\
		static void AndNot(DataT value){REG_NAME &= (DataT)(~value);}\
		static void Xor(DataT value){REG_NAME ^= (DataT)(value);}\
		static void AndOr(DataT andMask, DataT orMask){REG_NAME = (DataT)((REG_NAME & andMask) | orMask);}\
		template <int Bit>	\
		static void BitSet(){ REG_NAME|=(DataT)(1<<Bit);}\
		template <int Bit>	\
		static void BitClear(){ REG_NAME&= (DataT)~(1<<Bit);}\
		template <int Bit>	\
		static void BitToggle(){ REG_NAME ^= (DataT)(1<<Bit);}\
		template<int Bit>\
		static bool BitIsSet() {return REG_NAME & (DataT)(1 << Bit);}\
		template<int Bit>\
		static bool BitIsClear() {return !(REG_NAME & (DataT)(1 << Bit));}\
	}

#define I_REG_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE) \
	struct CLASS_NAME\
	{\
		typedef DATA_TYPE DataT;\
		static DataT Get(){return REG_NAME;}\
	}
/*
Dполне корректно. Только надо убедиться, что компилятор генерирует при этом более-менее оптимальный код. Компилятор может не оптимизировать ссылку и реализовать её посредствам указателя, в этом случае при обращении к регистрам будет лишнее чтение из памяти(этого указателя) и косвенное обращение к регистрам. Хотя на ARM-ах это не критично — всего несколько лишних тактов.
Когда регистры периферии упорядочены в структуру, я обычно использую такую обёртку:
*/
#define IO_STRUCT_WRAPPER(STRUCT_PTR, CLASS_NAME, STRUCT_TYPE) \
	struct CLASS_NAME\
	{\
		typedef STRUCT_TYPE DataT;\
		STRUCT_TYPE* operator->(){return ((STRUCT_TYPE *)(STRUCT_PTR));}\
	}

#define IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE, BITFIELD_OFFSET, BITFIELD_LENGTH) \
	struct CLASS_NAME\
	{\
		typedef DATA_TYPE DataT;\
		static const DataT Mask = ((DataT(1u) << BITFIELD_LENGTH) - 1);\
		static DataT Get(){return (REG_NAME >> BITFIELD_OFFSET) & Mask;}\
		static void Set(DataT value){REG_NAME = (REG_NAME & ~(Mask << BITFIELD_OFFSET)) | ((value & Mask) << BITFIELD_OFFSET);}\
	}


// A stub for IO register
// Any data written is ignored
// Always reads as zero
template<class DATA_TYPE = unsigned char>
struct NullReg {
	typedef DATA_TYPE DataT;
	static DataT Get() {return DataT(0);}
	static void Set(DataT) {}
	static void Or(DataT) {}
	static void And(DataT) {}
	static void Xor(DataT) {}
	static void AndOr(DataT, DataT) {}
	template <int Bit>
	static void BitSet() { }
	template <int Bit>
	static void BitClear() {}
	template <int Bit>
	static void BitToggle() {  }
	template<int Bit>
	static bool BitIsSet() {return false;}
	template<int Bit>
	static bool BitIsClear() {return true;}
};

