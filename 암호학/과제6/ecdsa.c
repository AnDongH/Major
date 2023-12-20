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
#include "ecdsa.h"
#include "sha2.h"
#include <gmp.h>
#include <string.h>

mpz_t p, n, Gx, Gy;


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

// num이 몇비트로 이루어져 있는지 확인
int countBits(size_t num) {
    int count = 0;
    while (num > 0) {
        count += 1;  // num의 가장 오른쪽 비트가 1인지 확인
        num >>= 1;        // num을 한 비트 오른쪽으로 시프트
    }   
    return count;
}


/*
 * Initialize 256 bit ECDSA parameters
 * 시스템파라미터 p, n, G의 공간을 할당하고 값을 초기화한다.
 */
void ecdsa_p256_init(void)
{
	// p, n, G mpz 초기화
	mpz_init_set_str(p, "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF", 16);
	mpz_init_set_str(n, "FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551", 16);
	mpz_init_set_str(Gx, "6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296", 16);
	mpz_init_set_str(Gy, "4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5", 16);

}

/*
 * Clear 256 bit ECDSA parameters
 * 할당된 파라미터 공간을 반납한다.
 */
void ecdsa_p256_clear(void)
{
	// p, n, G mpz 반납
	mpz_clears(p, n, Gx, Gy, NULL);
}

// P = Q일 때 계산
void scalar_doubling(mpz_t x1, mpz_t y1) 
{
	// 들어온 값이 0이면 리턴
	if (mpz_cmp_ui(x1, 0) == 0 && mpz_cmp_ui(y1, 0) == 0) return;

	// 변수 초기화
	mpz_t lambda, x3, y3, inv;
	mpz_inits(lambda, x3, y3, inv, NULL);

	// 람다 구하기
	// (3x1^2 - 3) / 2y1
	mpz_pow_ui(lambda, x1, 2);
	mpz_mul_ui(lambda, lambda, 3);
	mpz_sub_ui(lambda, lambda, 3);
	mpz_mul_ui(inv, y1, 2);
	mpz_invert(inv, inv, p);
	mpz_mul(lambda, lambda, inv);
	mpz_mod(lambda, lambda, p);
	

	// x3
	// lambda^2 - 2x1
	mpz_pow_ui(x3, lambda, 2);
	mpz_sub(x3, x3, x1);
	mpz_sub(x3, x3, x1);
	mpz_mod(x3, x3, p);

	// y3
	// lambda * (x1 - x3) - y1
	mpz_sub(y3, x1, x3);
	mpz_mul(y3, lambda, y3);
	mpz_sub(y3, y3, y1);
	mpz_mod(y3, y3, p);

	// x1에 값 양도
	mpz_set(x1, x3);
	mpz_set(y1, y3);

	// 반납
	mpz_clears(lambda, x3, y3, inv, NULL);
}


// P != Q 일 때 계산
void scalar_add(mpz_t x1, mpz_t y1, mpz_t x2, mpz_t y2)
{
	// P = Q라면 해당 계산으로
	if (mpz_cmp(x1, x2) == 0 && mpz_cmp(y1, y2) == 0) return scalar_doubling(x1, y1);
	// x1쪽이 0 x2쪽이 0이 아니라면 더해주기
	// x2쪽이 0 x1쪽이 0이 아니라면 그냥 냅두기(더해주나 마나)
    else if ((mpz_cmp_ui(x1, 0) == 0 && mpz_cmp_ui(y1, 0) == 0 ) || (mpz_cmp_ui(x2, 0) == 0 && mpz_cmp_ui(y2, 0) == 0 )) {
    	if (mpz_cmp_ui(x1, 0) == 0 && mpz_cmp_ui(y1, 0) == 0) {
            mpz_add(x1, x1, x2);
            mpz_add(y1, y1, y2);
        }
        return;
    }

	// 변수 초기화
	mpz_t lambda, x3, y3, inv;
	mpz_inits(lambda, x3, y3, inv, NULL);

	// 람다
	// (y2 - y1) / (x2 - x1)
	mpz_sub(lambda, y2, y1);
	mpz_sub(inv, x2, x1);
	mpz_invert(inv, inv, p);
	mpz_mul(lambda, lambda, inv);
	mpz_mod(lambda, lambda, p);

	// x3
	// lambda^2 - x1 - x2
	mpz_pow_ui(x3, lambda, 2);
	mpz_sub(x3, x3, x1);
	mpz_sub(x3, x3, x2);
	mpz_mod(x3, x3, p);

	// y3
	// lambda * (x1 - x3) - y1
	mpz_sub(y3, x1, x3);
	mpz_mul(y3, lambda, y3);
	mpz_sub(y3, y3, y1);
	mpz_mod(y3, y3, p);

	// x1에 값 양도
	mpz_set(x1, x3);
	mpz_set(y1, y3);
	
	// 반납
	mpz_clears(lambda, x3, y3, inv, NULL);
}


// Q = dG, (x1, y1) = kG 꼴을 만드는 함수
void multiple_add(mpz_t d, mpz_t x1, mpz_t y1, mpz_t x2, mpz_t y2)
{
	// 변수 초기화
	mpz_t tx, ty;
	mpz_inits(tx, ty, NULL);

	// 값 설정
	mpz_set(tx, x2);
	mpz_set(ty, y2);

	// d값이 쉬프트 연산으로 0보다 큰 동안
	while (mpz_cmp_ui(d, 0) > 0){
		// d의 0번째 비트가 1이냐?
		// 비트 1이면 더해주기
		if (mpz_tstbit(d, 0) == 1) scalar_add(x1, y1, tx, ty);

		// 쉬프트 연산
		mpz_fdiv_q_2exp(d, d, 1);
		// 값 누적
		scalar_doubling(tx, ty);
	}
	
	// 반납
	mpz_clears(tx, ty, NULL);
}


/*
 * ecdsa_p256_key() - generates Q = dG
 * 사용자의 개인키와 공개키를 무작위로 생성한다.
 */
void ecdsa_p256_key(void *d, ecdsa_p256_t *Q)
{
	// 변수 설정
	mpz_t dd, Qx, Qy, t;
    gmp_randstate_t state;
    gmp_randinit_default(state);
	
	// 시드 설정
    gmp_randseed_ui(state, arc4random());
    
	// 초기화
	mpz_inits(dd, Qx, Qy, t, NULL);

	// 1 ~ n-1사이에서 랜덤 추출 -> d 구하기
	while(mpz_cmp_ui(dd, 0) == 0){
		mpz_urandomm(dd, state, n);
	}

	// 임시 설정 -> 함수에 넣으면 값이 바뀌기에
    mpz_set(t, dd);

	// Q = dG 연산
    multiple_add(t, Qx, Qy, Gx, Gy);

	// 구한 값 넣어주기
    mpz_export(d, NULL, 1, ECDSA_P256/8, 1, 0, dd);
    mpz_export(Q->x, NULL, 1, ECDSA_P256/8, 1, 0, Qx);
    mpz_export(Q->y, NULL, 1, ECDSA_P256/8, 1, 0, Qy);

	// 반납
    mpz_clears(dd, Qx, Qy, t, NULL);
}

/*
 * ecdsa_p256_sign(msg, len, d, r, s) - ECDSA Signature Generation
 * 길이가 len 바이트인 메시지 m을 개인키 d로 서명한 결과를 r, s에 저장한다.
 * sha2_ndx는 사용할 SHA-2 해시함수 색인 값으로 SHA224, SHA256, SHA384, SHA512,
 * SHA512_224, SHA512_256 중에서 선택한다. r과 s의 길이는 256비트이어야 한다.
 * 성공하면 0, 그렇지 않으면 오류 코드를 넘겨준다.
 */
int ecdsa_p256_sign(const void *msg, size_t len, const void *d, void *_r, void *_s, int sha2_ndx)
{
	// 변수 설정
	int hLen = sha_len(sha2_ndx);
    mpz_t dd, ee, r, s, k, x1, y1, k_inv;
    gmp_randstate_t state;
 
	// 해시 변수
	unsigned char e[hLen];
	unsigned char cutE[ECDSA_P256/8];

	// 해시 못할 크기면 에러
	if (sha2_ndx == SHA224 || sha2_ndx == SHA256){
		if (countBits(len) > 61) return ECDSA_MSG_TOO_LONG;
	}
	else {
		if (countBits(len) > 125) return ECDSA_MSG_TOO_LONG;
	}

	// 초기화
	mpz_inits(dd, ee, r, s, k, x1, y1, k_inv, NULL);
	gmp_randinit_default(state);
    gmp_randseed_ui(state, arc4random());
	mpz_import(dd, ECDSA_P256/8, 1, 1, 1, 0, d);

	// 해시 -> e
	sha_gen(sha2_ndx, msg, len, e);

	// e의 길이가 n의 길이(256비트) 보다 길면 뒷부분은 자른다.
	if (hLen * 8 > ECDSA_P256){
		memcpy(cutE, e, sizeof(unsigned char)*(ECDSA_P256/8));
		mpz_import(ee, ECDSA_P256/8, 1, 1, 1, 0, cutE);
	}	
	else 
		mpz_import(ee, hLen, 1, 1, 1, 0, e);

	// 원하는 값 나올 때 까지 반복
    while(1) {
        do{
			// k 1 ~ n-1 추출
            mpz_urandomm(k, state, n);
        } while(mpz_cmp_ui(k, 0) == 0);

		// k 역원
        mpz_invert(k_inv, k, n);

		//  (x1, y1) = kG
        multiple_add(k, x1, y1, Gx, Gy);
        
		// r = x1 mod n
		mpz_mod(r, x1, n);

		// r 0이면 다시 반복
        if (mpz_cmp_ui(r, 0) == 0) {
            continue;
        }

		// s = k^-1(e + rd) mod n
        mpz_mul(s, r, dd);
        mpz_add(s, s, ee);
        mpz_mul(s, s, k_inv);
        mpz_mod(s, s, n);

		// s = 0이면 다시 반복
        if (mpz_cmp_ui(s, 0) == 0) {
            continue;
        }
        break;
    }

	// 값 넣어주기
    mpz_export(_s, NULL, 1, ECDSA_P256/8, 1, 0, s);
    mpz_export(_r, NULL, 1, ECDSA_P256/8, 1, 0, r);

	// 반납
    mpz_clears(dd, ee, r, s, k, x1, y1, k_inv, NULL);
    

	return 0;


}

/*
 * ecdsa_p256_verify(msg, len, Q, r, s) - ECDSA signature veryfication
 * It returns 0 if valid, nonzero otherwise.
 * 길이가 len 바이트인 메시지 m에 대한 서명이 (r,s)가 맞는지 공개키 Q로 검증한다.
 * 성공하면 0, 그렇지 않으면 오류 코드를 넘겨준다.
 */
int ecdsa_p256_verify(const void *msg, size_t len, const ecdsa_p256_t *_Q, const void *_r, const void *_s, int sha2_ndx)
{

	// 변수 설정
	int hLen = sha_len(sha2_ndx);
	mpz_t r, s, ee, u, a1, b1, a2, b2, Qx, Qy;

	// 해시 변수
	unsigned char e[hLen];
	unsigned char cutE[ECDSA_P256/8];

	// 해시 못할 크기면 리턴
	if (sha2_ndx == SHA224 || sha2_ndx == SHA256){
		if (countBits(len) > 61) return ECDSA_MSG_TOO_LONG;
	}
	else {
		if (countBits(len) > 125) return ECDSA_MSG_TOO_LONG;
	}

	// 초기화
    mpz_inits(r, s, ee, u, a1, b1, a2, b2, Qx, Qy, NULL);
    mpz_import(r, ECDSA_P256/8, 1, 1, 1, 0, _r);
    mpz_import(s, ECDSA_P256/8, 1, 1, 1, 0, _s);

	// r과s가 1 ~ n-1 사이 없으면 잘못된 서명
	if (mpz_cmp_ui(r,1) < 0 || 
	    !(mpz_cmp(r, n) < 0) ||
		mpz_cmp_ui(s, 1) < 0 ||
		!(mpz_cmp(s, n) < 0)) {

		mpz_clears(r, s, ee, u, a1, b1, a2, b2, Qx, Qy, NULL);
		return ECDSA_SIG_INVALID;
	}

	// 해시 -> e를 구한다.
    sha_gen(sha2_ndx, msg, len, e);

    // e의 길이가 n의 길이(256비트) 보다 길면 뒷부분은 자른다.
    if (hLen * 8 > ECDSA_P256){
        memcpy(cutE, e, sizeof(unsigned char)*(ECDSA_P256/8));
		mpz_import(ee, ECDSA_P256/8, 1, 1, 1, 0, cutE);
    }
	else
    	mpz_import(ee, hLen, 1, 1, 1, 0, e);
	
	// s^-1
	mpz_import(s, ECDSA_P256/8, 1, 1, 1, 0, _s);
    mpz_invert(s, s, n);

	// u1 = es^-1 mod n 
    mpz_mul(u, s, ee);
    mpz_mod(u, u, n);

	// u1G = (a1, b1)
    multiple_add(u, a1, b1, Gx, Gy);

    // u2 = rs^-1 mod n
    mpz_import(r, ECDSA_P256/8, 1, 1, 1, 0, _r);
    mpz_mul(u, s, r);
    mpz_mod(u, u, n);

    // u2Q = (a2, b2)
    mpz_import(Qx, ECDSA_P256/8, 1, 1, 1, 0, _Q->x);
    mpz_import(Qy, ECDSA_P256/8, 1, 1, 1, 0, _Q->y);
    multiple_add(u, a2, b2, Qx, Qy);	

	// (x1, y1) = u1G + u2Q -> a1, b1에 값 양도
	scalar_add(a1, b1, a2, b2);

	// 무한원점일시 잘못된 서명
	if (mpz_cmp_ui(a1, 0) == 0 && mpz_cmp_ui(b1, 0) == 0) {
        mpz_clears(r, s, ee, u, a1, b1, a2, b2, Qx, Qy, NULL);
        return ECDSA_SIG_INVALID;
    }

	// r = x1 mod n
    mpz_mod(a1, a1, n);

	// 원래 r과 값이 다르다면 잘못된 서명
	if (mpz_cmp(r, a1) != 0) {
        mpz_clears(r, s, ee, u, a1, b1, a2, b2, Qx, Qy, NULL);
        return ECDSA_SIG_MISMATCH;
    }

	// 반납
	mpz_clears(r, s, ee, u, a1, b1, a2, b2, Qx, Qy, NULL);

	return 0;

}








