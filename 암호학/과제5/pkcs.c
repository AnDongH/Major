/*
 * Copyright(c) 2020-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 자는 이 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */
#ifdef __linux__
#include <bsd/stdlib.h>
#elif __APPLE__
#include <stdlib.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include <gmp.h>
#include <math.h>
#include "pkcs.h"
#include "sha2.h"

void MGF1(const unsigned char *, size_t, size_t, int, unsigned char*);
void I2OSP(size_t, unsigned char*);
int countBits(size_t);
static void sha_gen(int sha2_ndx, const unsigned char *message, unsigned int len, unsigned char *digest);
static int sha_len(int sha2_ndx);

/*
 * rsa_generate_key() - generates RSA keys e, d and n in octet strings.
 * If mode = 0, then e = 65537 is used. Otherwise e will be randomly selected.
 * Carmichael's totient function Lambda(n) is used.
 */
void rsa_generate_key(void *_e, void *_d, void *_n, int mode)
{
    mpz_t p, q, lambda, e, d, n, gcd;
    gmp_randstate_t state;
    
    /*
     * Initialize mpz variables
     */
    mpz_inits(p, q, lambda, e, d, n, gcd, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, arc4random());
    /*
     * Generate prime p and q such that 2^(RSAKEYSIZE-1) <= p*q < 2^RSAKEYSIZE
     */
    do {
        do {
            mpz_urandomb(p, state, RSAKEYSIZE/2);
            mpz_setbit(p, 0);
            mpz_setbit(p, RSAKEYSIZE/2-1);
        } while (mpz_probab_prime_p(p, 50) == 0);
        do {
            mpz_urandomb(q, state, RSAKEYSIZE/2);
            mpz_setbit(q, 0);
            mpz_setbit(q, RSAKEYSIZE/2-1);
        } while (mpz_probab_prime_p(q, 50) == 0);
        /*
         * If we select e = 65537, it should be relatively prime to Lambda(n)
         */
        if (mode == 0) {
            mpz_sub_ui(p, p, 1);
            if (mpz_gcd_ui(gcd, p, 65537) != 1)
                continue;
            else
                mpz_add_ui(p, p, 1);
            mpz_sub_ui(q, q, 1);
            if (mpz_gcd_ui(gcd, q, 65537) != 1)
                continue;
            else
                mpz_add_ui(q, q, 1);
        }
        mpz_mul(n, p, q);
    } while (!mpz_tstbit(n, RSAKEYSIZE-1));
    /*
     * Generate e and d using Lambda(n)
     */
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_lcm(lambda, p, q);
    if (mode == 0)
        mpz_set_ui(e, 65537);
    else do {
        mpz_urandomb(e, state, RSAKEYSIZE);
        mpz_gcd(gcd, e, lambda);
    } while (mpz_cmp(e, lambda) >= 0 || mpz_cmp_ui(gcd, 1) != 0);
    mpz_invert(d, e, lambda);
    /*
     * Convert mpz_t values into octet strings
     */
    mpz_export(_e, NULL, 1, RSAKEYSIZE/8, 1, 0, e);
    mpz_export(_d, NULL, 1, RSAKEYSIZE/8, 1, 0, d);
    mpz_export(_n, NULL, 1, RSAKEYSIZE/8, 1, 0, n);
    /*
     * Free the space occupied by mpz variables
     */
    mpz_clears(p, q, lambda, e, d, n, gcd, NULL);
}

/*
 * rsa_cipher() - compute m^k mod n
 * If m >= n then returns PKCS_MSG_OUT_OF_RANGE, otherwise returns 0 for success.
 */
static int rsa_cipher(void *_m, const void *_k, const void *_n)
{
    mpz_t m, k, n;
    
    /*
     * Initialize mpz variables
     */
    mpz_inits(m, k, n, NULL);
    /*
     * Convert big-endian octets into mpz_t values
     */
    mpz_import(m, RSAKEYSIZE/8, 1, 1, 1, 0, _m);
    mpz_import(k, RSAKEYSIZE/8, 1, 1, 1, 0, _k);
    mpz_import(n, RSAKEYSIZE/8, 1, 1, 1, 0, _n);
    /*
     * Compute m^k mod n
     */
    if (mpz_cmp(m, n) >= 0) {
        mpz_clears(m, k, n, NULL);
        return PKCS_MSG_OUT_OF_RANGE;
    }
    mpz_powm(m, m, k, n);
    /*
     * Convert mpz_t m into the octet string _m
     */
    mpz_export(_m, NULL, 1, RSAKEYSIZE/8, 1, 0, m);
    /*
     * Free the space occupied by mpz variables
     */
    mpz_clears(m, k, n, NULL);
    return 0;
}

/*
 * rsaes_oaep_encrypt() - RSA encrytion with the EME-OAEP encoding method
 * 길이가 len 바이트인 메시지 m을 공개키 (e,n)으로 암호화한 결과를 c에 저장한다.
 * label은 데이터를 식별하기 위한 라벨 문자열로 NULL을 입력하여 생략할 수 있다.
 * sha2_ndx는 사용할 SHA-2 해시함수 색인 값으로 SHA224, SHA256, SHA384, SHA512,
 * SHA512_224, SHA512_256 중에서 선택한다. c의 크기는 RSAKEYSIZE와 같아야 한다.
 * 성공하면 0, 그렇지 않으면 오류 코드를 넘겨준다.
 */

int rsaes_oaep_encrypt(const void *m, size_t mLen, const void *label, const void *e, const void *n, void *c, int sha2_ndx) {
    
	// 변수 선언
	int k = RSAKEYSIZE / 8;
    int hLen = sha_len(sha2_ndx);
	int padding_size = k - mLen - 2 * hLen - 2;
	int DBlength = k - hLen - 1;
	int lLen = strlen(label);
	int maskLen = ceil((double)DBlength/hLen)*hLen;

	// 8비트 단위로 계산해주는 매우 큰 값들
	unsigned char lHash[hLen];
	unsigned char DB[DBlength];
	unsigned char seed[hLen];
	unsigned char dbMask[maskLen];
	unsigned char maskedDB[DBlength];
	unsigned char seedMask[hLen];
	unsigned char maskedSeed[hLen];
	unsigned char EM[k];	

	// 문서에 따라 해당 조건이면 return PKCS_MSG_TOO_LONG
    if (mLen > k - 2 * hLen - 2) {
        return PKCS_MSG_TOO_LONG;
    }

	// 라벨의 길이가 해시하기 너무 크면 return PKCS_LABEL_TOO_LONG 
	if (sha2_ndx == SHA224 || sha2_ndx == SHA256){
		if (countBits(lLen) > 61) return PKCS_LABEL_TOO_LONG;
	}
	else {
		if (countBits(lLen) > 125) return PKCS_LABEL_TOO_LONG;
	}

	// 라벨의 해시값 구하기
    sha_gen(sha2_ndx, label, lLen, lHash);

	// DB 구성하기
    memcpy(DB, lHash, hLen);
    memset(DB + hLen, 0x00, padding_size);
    DB[hLen + padding_size] = 0x01;
    memcpy(DB + hLen + padding_size + 1, m, mLen);

	// seed 랜덤으로 hLen 바이트만큼 뽑기
    arc4random_buf(seed, hLen);
    
	// seed MGF해서 dbMask 구하기
    MGF1(seed, hLen, DBlength, sha2_ndx, dbMask);

	// DB와 dbMask를 XOR해서 maskedDB 구하기
    for (int i = 0; i < DBlength; i++) {
        maskedDB[i] = dbMask[i] ^ DB[i];
    }

	// maskedDB MGF해서 seedMask구하기
    MGF1(maskedDB, DBlength, hLen, sha2_ndx, seedMask);
    
	// seedMask와 seed XOR해서 maskedSeed 구하기
    for (int i = 0; i < hLen; i++) {
        maskedSeed[i] = seedMask[i] ^ seed[i];
    }

	// EM 구하기
    EM[0] = 0x00; // 첫 바이트는 0x00
    memcpy(EM + 1, maskedSeed, hLen);
    memcpy(EM + 1 + hLen, maskedDB, DBlength);

	// EM 공개키로 암호화
    if (rsa_cipher(EM, e, n) == 1)
        return PKCS_MSG_OUT_OF_RANGE;

	// c에 복사
    memcpy(c, EM, k);

    return 0;
}
/*
 * rsaes_oaep_decrypt() - RSA decrytion with the EME-OAEP encoding method
 * 암호문 c를 개인키 (d,n)을 사용하여 원본 메시지 m과 길이 len을 회복한다.
 * label과 sha2_ndx는 암호화할 때 사용한 것과 일치해야 한다.
 * 성공하면 0, 그렇지 않으면 오류 코드를 넘겨준다.
 */
int rsaes_oaep_decrypt(void *m, size_t *mLen, const void *label, const void *d, const void *n, const void *c, int sha2_ndx)
{

	// 변수 선언
    int k = RSAKEYSIZE / 8;
    int hLen = sha_len(sha2_ndx);
	int DBlength = k - hLen - 1;
	int lLen = strlen(label);
	int pos = hLen;
	int maskLen = ceil((double)DBlength/hLen)*hLen;

	// 8비트 단위 매우 큰 수 정의
    unsigned char temC[k];
	unsigned char maskedSeed[hLen];
	unsigned char maskedDB[DBlength];
	unsigned char seedMask[hLen];
	unsigned char seed[hLen];
	unsigned char dbMask[maskLen];
	unsigned char DB[DBlength];
	unsigned char lHash[hLen];

	// 라벨 길이가 해시하기 너무 길다면 return PKCS_LABEL_TOO_LONG
	if (sha2_ndx == SHA224 || sha2_ndx == SHA256){
		if (countBits(lLen) > 61) return PKCS_LABEL_TOO_LONG;
	}
	else {
		if (countBits(lLen) > 125) return PKCS_LABEL_TOO_LONG;
	}


	// const 이므로 temC에 복사해서 복호화 진행
    memcpy(temC, c, sizeof(unsigned char)*k);

	// 복호화-> EM
	if(rsa_cipher(temC, d, n) != 0) return PKCS_MSG_OUT_OF_RANGE;
   
	// 첫 바이트가 0x00이 아니면 return PKCS_INITIAL_NONZERO
	if (temC[0] != 0x00) return PKCS_INITIAL_NONZERO;
	
	// EM에서 maskedSeed 구하기
    memcpy(maskedSeed, temC + 1, hLen);
    
	// EM에서 maskedDB 구하기
	memcpy(maskedDB, temC + 1 + hLen, DBlength);

	// maskedDB MGF해서 seedMask구하기
    MGF1(maskedDB, DBlength, hLen, sha2_ndx, seedMask);
	
	// seedMask와 maskedSeed XOR해서 seed구하기
    for (int i = 0; i < hLen; i++) {
        seed[i] = seedMask[i] ^ maskedSeed[i];
    }

	// seed MGF해서 dbMask구하기
    MGF1(seed, hLen, DBlength, sha2_ndx, dbMask);


	// dbMask와 maskedDB XOR해서 DB 구하기
    for (int i = 0; i < DBlength; i++) {
        DB[i] = dbMask[i] ^ maskedDB[i];
    }

	// 라벨 해시
    sha_gen(sha2_ndx, label, lLen, lHash);

	// 패딩 다음의 0x01 위치 구하기
	while(DB[pos] == 0x00) pos++;

	// 평문 길이 복원
    *mLen = DBlength - pos - 1;

	// 평문 길이 체크
	if (*mLen > k - 2 * hLen - 2) {
        return PKCS_MSG_TOO_LONG;
    }

	// 위치 맞는지 확인
    if (DB[pos] != 0x01)
        return PKCS_INVALID_PS;

	// 해시값 비교
    for (int i = 0; i < hLen; i++) {
        if (lHash[i] != DB[i]) {
			
			return PKCS_HASH_MISMATCH;
		}
    }

	// 평문 복원
    memcpy(m, DB+pos+1, sizeof(unsigned char)* (*mLen));


    return 0;
}


/*
 * rsassa_pss_sign - RSA Signature Scheme with Appendix
 * 길이가 len 바이트인 메시지 m을 개인키 (d,n)으로 서명한 결과를 s에 저장한다.
 * s의 크기는 RSAKEYSIZE와 같아야 한다. 성공하면 0, 그렇지 않으면 오류 코드를 넘겨준다.
 */
int rsassa_pss_sign(const void *m, size_t mLen, const void *d, const void *n, void *s, int sha2_ndx)
{
    
	// 변수 선언
	int k = RSAKEYSIZE / 8;
    int hLen = sha_len(sha2_ndx);
    int DBlength = k - hLen - 1;
    int PS_len = DBlength - hLen - 1;
	int maskLen = ceil((double)DBlength/hLen)*hLen;

	// 8비트 단위의 큰 값들 선언
    unsigned char mHash[hLen];
    unsigned char M_P[8 + 2 * hLen];
    unsigned char M_P_Hash[hLen];
    unsigned char DB[DBlength];
    unsigned char dbMask[maskLen];
    unsigned char maskedDB[DBlength];
    unsigned char EM[k];
    unsigned char salt[hLen];

	// 메시지가 해시하기 위한 길이를 만족하는지 확인
	if (sha2_ndx == SHA224 || sha2_ndx == SHA256){
		if (countBits(mLen) > 61) return PKCS_MSG_TOO_LONG;
	}
	else {
		if (countBits(mLen) > 125) return PKCS_MSG_TOO_LONG;
	}

	// 문서상으로 해당 조건이면 return PKCS_HASH_TOO_LONG
	if (k < 2*hLen + 2) return PKCS_HASH_TOO_LONG;

    // m을 해시해서 mHash구하기
    sha_gen(sha2_ndx, (unsigned char *)m, mLen, mHash);

    // salt 구하기
    arc4random_buf(salt, sizeof(unsigned char) * hLen);
    
    // M 프라임 구성하기
    memset(M_P, 0x00, sizeof(unsigned char) * 8);
    memcpy(M_P + 8, mHash, sizeof(unsigned char) * hLen);
    memcpy(M_P + (8 + hLen), salt, sizeof(unsigned char) * hLen);

    // M 프라임 해시해서 M_P_Hash 구하기
    sha_gen(sha2_ndx, M_P, sizeof(M_P), M_P_Hash);

    // M_P_Hash MGF해서 dbMask 구하기
	MGF1(M_P_Hash, hLen, DBlength, sha2_ndx, dbMask);

	// DB 구성
    memset(DB, 0x00, sizeof(unsigned char) * PS_len);
    memset(DB + PS_len, 0x01, sizeof(unsigned char));
    memcpy(DB + (PS_len + 1), salt, sizeof(unsigned char) * hLen);

    // DB와 dbMask XOR 연산해서 maskedDB 구하기
    for (int i = 0; i < DBlength; i++)
        maskedDB[i] = DB[i] ^ dbMask[i];

    // EM 구성
    memcpy(EM, maskedDB, DBlength);
    memcpy(EM + DBlength, M_P_Hash, sizeof(unsigned char) * hLen);
    memset(EM + (DBlength + hLen), 0xBC, sizeof(unsigned char));

	// EM의 맨 왼쪽 비트가 1이라면 0으로 바꿔주기
	if ((EM[0] >> 7) == 1) EM[0] = EM[0] ^ 0x80;

    // EM을 개인키로 서명하기
    if (rsa_cipher(EM, d, n) != 0) return PKCS_MSG_OUT_OF_RANGE;
    	
	// s에 복사
	memcpy(s, EM, sizeof(EM));
    return 0;
}


/*
 * rsassa_pss_verify - RSA Signature Scheme with Appendix
 * 길이가 len 바이트인 메시지 m에 대한 서명이 s가 맞는지 공개키 (e,n)으로 검증한다.
 * 성공하면 0, 그렇지 않으면 오류 코드를 넘겨준다.
 */
int rsassa_pss_verify(const void *m, size_t mLen, const void *e, const void *n, const void *s, int sha2_ndx)
{

	// 변수 선언
	int k = RSAKEYSIZE / 8;
	int hLen = sha_len(sha2_ndx);
	int DBlength = k - hLen - 1;
	int PS_len = DBlength - hLen - 1;
	int maskLen = ceil((double)DBlength/hLen)*hLen;

	// 8비트 단위의 큰 값들 선언
	unsigned char mHash[hLen];
	unsigned char M_p[8+hLen*2];
	unsigned char M_P_Hash[hLen];
	unsigned char EM_H[hLen];
	unsigned char salt[hLen];
	unsigned char DB[DBlength];
	unsigned char maskedDB[DBlength];
	unsigned char dbMask[maskLen];
	unsigned char temS[k];

	// m의 길이가 해시하기 너무 긴 지 확인
	if (sha2_ndx == SHA224 || sha2_ndx == SHA256){
		if (countBits(mLen) > 61) return PKCS_MSG_TOO_LONG;
	}
	else {
		if (countBits(mLen) > 125) return PKCS_MSG_TOO_LONG;
	}

	// 문서에 따라서 해당 조건이면 return PKCS_HASH_TOO_LONG
	if (k < 2*hLen + 2) return PKCS_HASH_TOO_LONG;

	// m을 해시해서 mHash 구하기
	sha_gen(sha2_ndx, m, mLen, mHash);
	memcpy(temS, s, sizeof(unsigned char)*k);

	// s를 검증해서 EM 구하기 
	if (rsa_cipher(temS,e,n) != 0) return PKCS_MSG_OUT_OF_RANGE;
	
	// EM의 마지막 바이트가 0xBC인지 확인
	if (temS[k-1] != 0xBC) return PKCS_INVALID_LAST;

	// EM의 가장 첫 비트가 0인지 확인
	if ((temS[0] >> 7) != 0) return PKCS_INVALID_INIT;

	// EM에서 해시값 뽑아내기
	memcpy(EM_H, temS+DBlength, sizeof(unsigned char)*hLen);

	// EM에서 maskedDB 뽑아내기
	memcpy(maskedDB, temS, sizeof(unsigned char)*(DBlength));
    
	// EM_H 해시해서 dbMask 구하기
	MGF1(EM_H, hLen, DBlength, sha2_ndx, dbMask);
	
	// maskedDB와 dbMask를 XOR 연산해서 DB 구하기
	for (int i = 0; i < DBlength; i++)
		DB[i] = maskedDB[i] ^ dbMask[i];

	// 만약 DB의 첫 비트가 1이면 0으로 바꿔주기
	// 이전 서명에서 maskedDB의 첫 비트를 변경해줬으니
	// 이를 XOR한 DB의 첫 비트는 0이 아닐 수도 있기에,
	if ((DB[0] >> 7) != 0) DB[0] = DB[0] ^ 0x80;	

	// DB의 패딩 부분과 0x01 이 일치하는지 확인
	for (int i = 0; i < PS_len; i++)
		if(DB[i] != 0x00) return PKCS_INVALID_PD2;
	if(DB[PS_len] != 0x01) return PKCS_INVALID_PD2;

	// DB에서 salt 뽑아내기
	memcpy(salt, DB+PS_len+1, sizeof(unsigned char)*hLen);

	// M 프라임 만들기
	memset(M_p, 0x00, sizeof(unsigned char)*8);
	memcpy(M_p + 8, mHash, sizeof(unsigned char)*hLen);
	memcpy(M_p + hLen + 8, salt, sizeof(unsigned char)*hLen);

	// M_P 해시해서 M_P_Hash 만들기
	sha_gen(sha2_ndx, M_p, 8+2*hLen, M_P_Hash);

	// 해시값 일치하는지 확인
	for (int i = 0; i < hLen; i++){
		if (M_P_Hash[i] != EM_H[i]){
       		 return PKCS_HASH_MISMATCH;
		}
	}

	return 0;
}

void MGF1(const unsigned char *mgfSeed, size_t seed_len, size_t mask_len, int
sha2_ndx, unsigned char *T){
	
	// 변수 선언
	size_t hLen;
	uint64_t s;

	// 값 구하기
	hLen = sha_len(sha2_ndx);
	s = hLen * 0x100000000;

	// 문서에 따라서 해당 조건이면 강제 종료
	if (mask_len > s) {
		printf("mask_len too long");
		exit(1);
	}
	
	
	// ceil((double)mask_len/hLen)만큼 반복해서 T에 해시값을 추가하기
	for (int i = 0; i < ceil((double)mask_len/hLen); i++){
		// 임시 변수 선언
		unsigned char c[4];
		unsigned char digest[hLen];
		unsigned char tem[seed_len + 4];

		// c 구하기 -> I2OSP(i, 4) 일때 알고리즘
		I2OSP(i, c);

		// tem = mgfSees || c
		memcpy(tem, mgfSeed, sizeof(unsigned char)*seed_len);
		memcpy(tem + seed_len, c, sizeof(unsigned char)*4);

		// tem 해시해서 digest에 넣기
		sha_gen(sha2_ndx, tem, seed_len+4, digest);

		// T = T || digest
		memcpy(T+hLen*i, digest, sizeof(unsigned char)*hLen);
	
	}	

}

void I2OSP(size_t x, unsigned char *t){

	// 주어진 값 x를 8비트 단위로 4개로 자르기
 	uint32_t x_tem = (uint32_t)x;
	t[0] = (unsigned char)((x_tem >> 24) & 0xFF);
	t[1] = (unsigned char)((x_tem >> 16) & 0xFF);
	t[2] = (unsigned char)((x_tem >> 8) & 0xFF);
	t[3] = (unsigned char)(x_tem & 0xFF);
}

// num이 몇비트로 이루어져 있는지 확인
int countBits(size_t num) {
    int count = 0;
    while (num > 0) {
        count += 1;  // num의 가장 오른쪽 비트가 1인지 확인
        num >>= 1;        // num을 한 비트 오른쪽으로 시프트
    }
    return count;
}


// 원하는 해시 함수 버전으로 해쉬값을 생성해준다.
static void sha_gen(int sha2_ndx, const unsigned char *message, unsigned int len, unsigned char *digest)
{

	// 함수 포인터 배열 -> 해시 함수들
    void (*f[6])(const unsigned char *, unsigned int, unsigned char *) = {
        sha224, sha256, sha384, sha512, sha512_224, sha512_256
    };

	// 정당한 값이 들어왔다면 해당 해시 함수로 해시
    if (sha2_ndx >= 0 && sha2_ndx <= 5)
        f[sha2_ndx](message, len, digest);

}

//  원하는 해시 값의 길이를 리턴한다.
static int sha_len(int sha2_ndx)
{
	// 미리 정의된 해시 길이의 배열
    int shalen_index[6] = {
        SHA224_DIGEST_SIZE, SHA256_DIGEST_SIZE, SHA384_DIGEST_SIZE,
        SHA512_DIGEST_SIZE, SHA224_DIGEST_SIZE, SHA256_DIGEST_SIZE
        };
    
	// 정당하지 못한 값이라면 리턴
    if (sha2_ndx > 5 || sha2_ndx < 0)
        return 1;
	// 해시 길이 리턴
    return (shalen_index[sha2_ndx]);
}

