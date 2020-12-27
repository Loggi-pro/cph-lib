#pragma once
#include <void/static_assert.h>
#ifdef STM32F303xC
#define FLASH_PAGE_COUNT 128
#define FLASH_PAGE_LAST FLASH_PAGE_COUNT-1
#endif
namespace cph {

	namespace Private {
		template<typename T>
struct is_pointer { static const bool value = false; };

template<typename T>
struct is_pointer<T*> { static const bool value = true; };
	}
	struct flash_address_t{
		uint32_t value;
	};
	class Flash {
		private:
			Flash() = delete;
			~Flash() = delete;
			static bool isReady() {
				return !(FLASH->SR & FLASH_SR_BSY);
			}

			static void _erase_all_pages() {
				FLASH->CR |=
				    FLASH_CR_MER; //Устанавливаем бит стирания ВСЕХ страниц
				FLASH->CR |= FLASH_CR_STRT; //Начать стирание

				while (!isReady()) // Ожидание готовности.. Хотя оно уже наверное ни к чему здесь...
				{ continue; }

				FLASH->CR&= FLASH_CR_MER;
			}
		public:
			static flash_address_t pages(uint8_t n) {
				return flash_address_t{FLASH_BASE+(n)*FLASH_PAGE_SIZE};
			}
			static void lock() {
				FLASH->CR |= FLASH_CR_LOCK;
			}

			static void unlock() {
				FLASH->KEYR = 0x45670123;
				FLASH->KEYR = 0xCDEF89AB;
			}
//pageAddress - любой адрес, принадлежащий стираемой странице
			static void erase(flash_address_t pageAddress) {
				while (!isReady()) { continue; }

				if (FLASH->SR & FLASH_SR_EOP) {
					FLASH->SR |= FLASH_SR_EOP;
				}

				FLASH->CR |= FLASH_CR_PER;
				FLASH->AR = pageAddress.value;
				FLASH->CR |= FLASH_CR_STRT;

				while (!(FLASH->SR & FLASH_SR_EOP)) { continue; }

				FLASH->SR = FLASH_SR_EOP;
				FLASH->CR &= ~FLASH_CR_PER;
			}

//data - указатель на записываемые данные
//address - адрес во flash
//count - количество записываемых байт, должно быть кратно 2
			static void write(uint32_t address,uint8_t* data,  uint32_t count) {
				while (!isReady()) { continue; }

				if (FLASH->SR & FLASH_SR_EOP) {
					FLASH->SR |= FLASH_SR_EOP;
				}

				FLASH->CR |= FLASH_CR_PG;

				for (uint32_t i = 0; i < count; i += 2) {
					*(__IO uint16_t*)(address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];

					while (!(FLASH->SR & FLASH_SR_EOP)) { continue; }

					FLASH->SR = FLASH_SR_EOP;
				}

				FLASH->CR &= ~(FLASH_CR_PG);
			}
			template <typename T>
			static void write(int32_t address,const T& var){
				VOID_STATIC_ASSERT(sizeof(T)%2==0);
				VOID_STATIC_ASSERT(Private::is_pointer<T>::value == false);//This must not be a pointer
				uint8_t* ptr = (uint8_t*) &var;
				write(address,ptr,sizeof(T));
			}

			static uint32_t read(uint32_t address){
				return (*(__IO uint32_t*) address);
			}
			template <typename T>
			static T* read(uint32_t address,T* var){
				const uint16_t count = sizeof(T)/sizeof(uint32_t);
				const uint16_t last_bytes = sizeof(T)%sizeof(uint32_t);
				uint32_t* ptr = (uint32_t*) var;
				for (uint16_t i = 0; i < count; i++)
				{
					*ptr = read(address);
					address+=sizeof(uint32_t);
					ptr++;
				}
				if (last_bytes==0) return var;
				uint8_t* bytePtr = (uint8_t*)ptr;
					uint32_t value = read(address);	
				switch (last_bytes){
					case 3:*bytePtr++ = vd::lowByte(vd::hiWord(value));
					case 2:*bytePtr++ = vd::hiByte(value)>>8;
					case 1:*bytePtr = vd::lowByte(value);
					break;
				}

				return var;
				

								
			}			
	};
}