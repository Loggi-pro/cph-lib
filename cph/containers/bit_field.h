#pragma once
#include <void/optional.h>
namespace cph {
template <uint32_t size>
using bit_array_t = u08[(((size)+(8-1))/8)];


struct bitfield_t {
		static void setBit(u08* arr,u16 n){
				u16 byte = n / 8;
				u16 bit = n % 8;
				arr[byte] |= (uint8_t)(1 << bit);
		}
		static void clearBit(u08* arr,u16 n){
			u16 byte = n / 8;
			u16 bit = n % 8;
			arr[byte] &= (uint8_t)~(1 << bit);
		}
		static u08 getBit(u08* arr,u16 n){
			u16 byte = n / 8;
			u16 bit = n % 8;
			return (arr[byte] & (uint8_t)(1 << bit)) > 0 ? 1 : 0;
		}
		static bool isBitSet(u08* arr,u16 n){
			u16 byte = n / 8;
			u16 bit = n % 8;

			if (arr[byte] == 0) { return false; }

			return (arr[byte] & (uint8_t)(1 << bit)) > 0 ? true : false;
		}
		//return true if finded;
		static vd::optional_t<uint16_t> getFirstCheckedBit(u08* arr,uint16_t size) {
			for (u16 i = 0; i < size; i++) {
			u08 byte = arr[i];

			if (byte == 0) { continue; }

			for (uint16_t j = 0; j < 8; j++) {
				if ((byte & 0x01) == 1) {
					return vd::optional_t<uint16_t>(i * 8 + j);
				}

				byte = (uint8_t)(byte>>1);
			}
		}

		return vd::optional_t<uint16_t>();
		}
		static uint16_t from2dIndex(u08 x, u08 y, u08 width){
			return x + (uint16_t)(width * y);
		}
		
		static int countSetBits(u08* arr,uint16_t size) {
			int count = 0;
			for (u16 i = 0; i < size; i++) {
				u08 a = arr[i];
				while (a) {
					count++;
					a &= (uint8_t)(a-1);
				}
			}
			return count;
		}
		
		
};
}

/*

static void printBits(u16 const size, void const* const ptr) {
	unsigned char* b = (unsigned char*)ptr;
	unsigned char byte;
	int i, j;

	for (i = size - 1; i >= 0; i--) {
		for (j = 7; j >= 0; j--) {
			byte = b[i] & (1 << j);
			byte >>= j;
			//printf("%ud", byte);
		}
	}

	//printf(" ");
}

void bf_print(Tbit_array ar[], u16 size) {
	for (int j = size - 1; j >= 0; j--)
	{ printBits(1, ar + j); }

	//printf("\n");
}


static u08 test_ar[] = { 0, 0, 0,0 };


static u32 get_val(){
	return (u32)((u32)(test_ar[3])<<24 | (u32)(test_ar[2])<<16 | (u32)(test_ar[1])<<8 |(u32)(test_ar[0]));
}


void _bf_test(){

printf("Check size:\n");
for (int i = 0; i <=24; i ++ )
	printf("%d/8=%d\n", i,bf_size(i));

printf("Test setbit:\n");
for (u16 i = 0;i<sizeof(test_ar);i++) test_ar[i] = 0;
bf_setbit(test_ar,10);
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,9),false,"");
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,10),true,"");
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,11),false,"");

for (u16 i = 0;i<sizeof(test_ar);i++) test_ar[i] = 0;
bf_setbit(test_ar,0);
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,0),true,"");
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,1),false,"");

for (u16 i = 0;i<sizeof(test_ar);i++) test_ar[i] = 0;
bf_setbit(test_ar,sizeof(test_ar)*8-1);
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,sizeof(test_ar)*8-1),true,"");
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,sizeof(test_ar)*8-2),false,"");


printf("Test blink:\n");
for (u16 i = 0;i<sizeof(test_ar);i++) test_ar[i] = 0;
bf_setbit(test_ar,10);
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,10),true,"");
bf_clearbit(test_ar,10);
TEST_ASSERT_EQUAL(bf_bit_is_set(test_ar,10),false,"");



printf("Test fill from right to left:\n");
for (u16 i = 0;i<sizeof(test_ar);i++) test_ar[i] = 0;
u32 counter=1;
for (u16 i = 0; i < sizeof(test_ar)* 8;i++){
	bf_setbit(test_ar, i);
	TEST_ASSERT_EQUAL(get_val(),counter,"");
	counter |=counter<<1;
}
printf("Test clear from right to left\n");
counter = 0xFFFFFFFF;

for (u16 i = 0; i < sizeof(test_ar)* 8; i++){
	bf_clearbit(test_ar, i);
	counter =counter<<1;

	TEST_ASSERT_EQUAL(get_val(),counter,"");
	if (i == sizeof(test_ar)*8-1) continue;
	u16 bit;
	bf_get_first_checked_bit(test_ar,sizeof(test_ar),&bit);
	TEST_ASSERT_EQUAL(bit,i+1,"bit must be checked");
}
printf("Test fil from left to right\n");

counter=0x80000000;
for (u16 i = (sizeof(test_ar))*8-1; i >=0; i--){
	bf_setbit(test_ar, i);
	u16 bit;
	bf_get_first_checked_bit(test_ar,sizeof(test_ar),&bit);
	TEST_ASSERT_EQUAL(get_val(),counter,"");
	TEST_ASSERT_EQUAL(bit,i,"bit must be checked");
	counter |= counter>>1;
	if (i==0) break;
}
printf("Test clear from left to right\n");
counter=0xFFFFFFFF;
for (u16 i = (sizeof(test_ar)) * 8-1; i >= 0; i--){
	bf_clearbit(test_ar, i);
	counter >>=1;
	TEST_ASSERT_EQUAL(get_val(),counter,"");
	if (i==0) break;
}
}
*/

