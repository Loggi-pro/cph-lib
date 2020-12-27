#pragma once


//»спользование гост:
/*
uint32_t key[8] = {0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777, 0x88888888};
uint32_t plain[2] = {0xAAAAAAAA, 0xBBBBBBBB};
uint32_t cipher[2];
uint32_t result[2];

//»нициализаци€ "криптор€дра"
gost_init();

//Ўифруем plain ключом key, результат шифровани€ помешаем в cipher
gostcrypt(plain, cipher, key);

//ƒешифруем cipher ключом key, результат шифровани€ помешаем в result
gostdecrypt(cipher, result, key);*/

class TCrypt {
		static unsigned char const k8[16];
		static unsigned char const k7[16];
		static unsigned char const k6[16];
		static unsigned char const k5[16];
		static unsigned char const k4[16];
		static unsigned char const k3[16];
		static unsigned char const k2[16];
		static unsigned char const k1[16];
		static unsigned char k87[256];
		static unsigned char k65[256];
		static unsigned char k43[256];
		static unsigned char k21[256];

		static inline uint32_t gost_f(uint32_t x) {
			/* Do substitutions */
			/* This is faster */
			x = (uint64_t)k87[x >> 24 & 255] << 24 |  (uint64_t)k65[x >> 16 & 255] << 16 |
			    k43[x >> 8 & 255] <<  8 | k21[x & 255];
			/* Rotate left 11 bits */
			return x << 11 | x >> (32 - 11);
		}

	public:

		//v Ч исходный текст состо€щий из двух слов по 32 бита, key Ч ключ состо€щий из четырех 32-битных слов
		//num_rounds Ч число циклов алгоритма (рекомендуетс€ 32)
		//num_rounds должно быть одинаковым дл€ шифровани€ и расшифровани€, если num_rounds==0 то ни шифровани€, ни расшифровани€ происходить не будет
		static void xtea_encipher(unsigned int num_rounds, uint32_t* v, uint32_t const* key);
		static void xtea_decipher(unsigned int num_rounds, uint32_t* v, uint32_t const* key);

		//√ќ—“ 28 147-89
		static void gost_init(void) {
			uint16_t i;

			for (i = 0; i < 256; i++) {
				k87[i] = k8[i >> 4] << 4 | k7[i & 15];
				k65[i] = k6[i >> 4] << 4 | k5[i & 15];
				k43[i] = k4[i >> 4] << 4 | k3[i & 15];
				k21[i] = k2[i >> 4] << 4 | k1[i & 15];
			}
		}


		/*
		 * The GOST standard defines the input in terms of bits 1..64, with
		 * bit 1 being the lsb of in[0] and bit 64 being the msb of in[1].
		 *
		 * The keys are defined similarly, with bit 256 being the msb of key[7].
		 */
		static void gostcrypt(uint32_t const in[2], uint32_t out[2], uint32_t const key[8]);

		/*
		 * The key schedule is somewhat different for decryption.
		 * (The key table is used once forward and three times backward.)
		 * You could define an expanded key, or just write the code twice,
		 * as done here.
		 */
		static void gostdecrypt(uint32_t const in[2], uint32_t out[2], uint32_t const key[8]);

};





//XTEA
void TCrypt::xtea_encipher(unsigned int num_rounds, uint32_t* v, uint32_t const* key) {
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;

	for (i = 0; i < num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}

	v[0] = v0; v[1] = v1;
}

void TCrypt::xtea_decipher(unsigned int num_rounds, uint32_t* v, uint32_t const* key) {
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta * num_rounds;

	for (i = 0; i < num_rounds; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}

	v[0] = v0; v[1] = v1;
}




/*
 * The GOST standard defines the input in terms of bits 1..64, with
 * bit 1 being the lsb of in[0] and bit 64 being the msb of in[1].
 *
 * The keys are defined similarly, with bit 256 being the msb of key[7].
 */
void TCrypt::gostcrypt(uint32_t const in[2], uint32_t out[2], uint32_t const key[8]) {
	register uint32_t n1, n2; /* As named in the GOST */
	n1 = in[0];
	n2 = in[1];
	/* Instead of swapping halves, swap names each round */
	n2 ^= gost_f(n1 + key[0]);
	n1 ^= gost_f(n2 + key[1]);
	n2 ^= gost_f(n1 + key[2]);
	n1 ^= gost_f(n2 + key[3]);
	n2 ^= gost_f(n1 + key[4]);
	n1 ^= gost_f(n2 + key[5]);
	n2 ^= gost_f(n1 + key[6]);
	n1 ^= gost_f(n2 + key[7]);
	n2 ^= gost_f(n1 + key[0]);
	n1 ^= gost_f(n2 + key[1]);
	n2 ^= gost_f(n1 + key[2]);
	n1 ^= gost_f(n2 + key[3]);
	n2 ^= gost_f(n1 + key[4]);
	n1 ^= gost_f(n2 + key[5]);
	n2 ^= gost_f(n1 + key[6]);
	n1 ^= gost_f(n2 + key[7]);
	n2 ^= gost_f(n1 + key[0]);
	n1 ^= gost_f(n2 + key[1]);
	n2 ^= gost_f(n1 + key[2]);
	n1 ^= gost_f(n2 + key[3]);
	n2 ^= gost_f(n1 + key[4]);
	n1 ^= gost_f(n2 + key[5]);
	n2 ^= gost_f(n1 + key[6]);
	n1 ^= gost_f(n2 + key[7]);
	n2 ^= gost_f(n1 + key[7]);
	n1 ^= gost_f(n2 + key[6]);
	n2 ^= gost_f(n1 + key[5]);
	n1 ^= gost_f(n2 + key[4]);
	n2 ^= gost_f(n1 + key[3]);
	n1 ^= gost_f(n2 + key[2]);
	n2 ^= gost_f(n1 + key[1]);
	n1 ^= gost_f(n2 + key[0]);
	/* There is no swap after the last round */
	out[0] = n2;
	out[1] = n1;
}



/*
 * The key schedule is somewhat different for decryption.
 * (The key table is used once forward and three times backward.)
 * You could define an expanded key, or just write the code twice,
 * as done here.
 */
void TCrypt::gostdecrypt(uint32_t const in[2], uint32_t out[2], uint32_t const key[8]) {
	register uint32_t n1, n2; /* As named in the GOST */
	n1 = in[0];
	n2 = in[1];
	n2 ^= gost_f(n1 + key[0]);
	n1 ^= gost_f(n2 + key[1]);
	n2 ^= gost_f(n1 + key[2]);
	n1 ^= gost_f(n2 + key[3]);
	n2 ^= gost_f(n1 + key[4]);
	n1 ^= gost_f(n2 + key[5]);
	n2 ^= gost_f(n1 + key[6]);
	n1 ^= gost_f(n2 + key[7]);
	n2 ^= gost_f(n1 + key[7]);
	n1 ^= gost_f(n2 + key[6]);
	n2 ^= gost_f(n1 + key[5]);
	n1 ^= gost_f(n2 + key[4]);
	n2 ^= gost_f(n1 + key[3]);
	n1 ^= gost_f(n2 + key[2]);
	n2 ^= gost_f(n1 + key[1]);
	n1 ^= gost_f(n2 + key[0]);
	n2 ^= gost_f(n1 + key[7]);
	n1 ^= gost_f(n2 + key[6]);
	n2 ^= gost_f(n1 + key[5]);
	n1 ^= gost_f(n2 + key[4]);
	n2 ^= gost_f(n1 + key[3]);
	n1 ^= gost_f(n2 + key[2]);
	n2 ^= gost_f(n1 + key[1]);
	n1 ^= gost_f(n2 + key[0]);
	n2 ^= gost_f(n1 + key[7]);
	n1 ^= gost_f(n2 + key[6]);
	n2 ^= gost_f(n1 + key[5]);
	n1 ^= gost_f(n2 + key[4]);
	n2 ^= gost_f(n1 + key[3]);
	n1 ^= gost_f(n2 + key[2]);
	n2 ^= gost_f(n1 + key[1]);
	n1 ^= gost_f(n2 + key[0]);
	out[0] = n2;
	out[1] = n1;
}








unsigned char const TCrypt::k8[16] = {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7 };
unsigned char const TCrypt::k7[16] = {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10 };
unsigned char const TCrypt::k6[16] = {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8 };
unsigned char const TCrypt::k5[16] = {7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15 };
unsigned char const TCrypt::k4[16] = {2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9 };
unsigned char const TCrypt::k3[16] = {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11 };
unsigned char const TCrypt::k2[16] = {4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1 };
unsigned char const TCrypt::k1[16] = {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7 };
unsigned char TCrypt::k87[256];
unsigned char TCrypt::k65[256];
unsigned char TCrypt::k43[256];
unsigned char TCrypt::k21[256];
