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
#include "mRSA.h"

/*
 * mod_add() - computes a + b mod m
 */
static uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m)
{
	// 임시 변수 선언 % m
	uint64_t r1 = a % m;
	uint64_t r2 = b % m;

	// 오버 플로 나지 않도록 조건
	return r1 < (m - r2) ? r1 + r2 : r1 - (m - r2);
}

/*
 * mod_mul() - computes a * b mod m
 */
static uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m)
{
	uint64_t r = 0;
	while (b > 0){
		// b 비트가 1이면 더하기
		if (b & 1) r = mod_add(r, a, m);
		b = b >> 1;
		// a*2값 누적
		a = mod_add(a,a,m);
	}
	return r;
}

/*
 * mod_pow() - computes a^b mod m
 */
static uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m)
{
	uint64_t r = 1;
	while (b > 0){
		// b 비트가 1이면 곱하기
		if (b & 1) r = mod_mul(r,a,m);
		b = b >> 1;
		// a^2값 누적
		a = mod_mul(a,a,m);
	} 
	return r;
}

/*
 * gcd() - Euclidean algorithm
 */
static uint64_t gcd(uint64_t a, uint64_t b)
{
 	 /* 
		 gcd(a, b) = gcd(b, a % b) 와 같음을 이용한 방식이다.
   		 재귀 호출은 일반적으로 스택에 계속해서 메모리를 쌓아가고
   		 다시 복귀주소로 리턴해주는 과정을 가지기에 특별한 경우가 아니라면
   		 반복문이 더 높은 성능을 발휘한다.

  		 매 반복마다 a 에는 b를 넣고, b에는 a % b를 넣는 과정을 진행한다.
  	*/ 	
	uint64_t tem;
	while(b != 0){
		tem = b;
		b = a % b;
		a = tem;
	}
	return a;
}

/*
 * mul_inv() - computes multiplicative inverse a^-1 mod m
 * It returns 0 if no inverse exist.
 */
static uint64_t mul_inv(uint64_t a, uint64_t m)
{
	// 변수 설정
	uint64_t d0 = a, d1 = m;
    uint64_t x0 = 1, x1 = 0, q, tem, t;
    int isNx0 = 0, isNx1 = 0, isNt = 0;
   	
	// d1 > 1동안
    while (d1 > 1){

		// d의 계산은 동일
    	q = d0 / d1; 
        tem = d0 - q * d1; 
        d0 = d1; 
        d1 = tem;
    
		// 임시 저장 변수
		t = x1; isNt = isNx1;
		
		// x0와 x1의 부호 다른지 같은지 비교
		if (isNx0 != isNx1){
			/*
		    	다르다면 + 를 해주고 isNx1 = isNx0으로 맞춰준다.
			 	
				만약 isNx0 = 0, isNx1 = 1 일 경우
				원래 연산은 - 이고 -의 -는 +이므로 +를 해준 다음
				isNx1 은 양수가 된다.

				만약 isNx0 = 1, isNx1 = 0일 경우
				원래 연산은 - 이고 이는  두 값을 더해준 다음
				음수로 만들어주는 것과 같기에 더해준 후
				isNx1 은 음수가 된다.
			*/
			
			x1 = x0 + q * x1;
			isNx1 = isNx0;
		}
		else{
			// 두 부호가 같다면 두 값을 비교해줘야 한다.	
			// x0가 더 큰 값이면 기존 연산과 같고 x1은 무조건 양수
			// q * x1이 더 크다면 반대로 계산해주고 x1은 무조건 음수
			x1 = (x0 > q * x1) ? x0 - q * x1 : q * x1 - x0;
			isNx1 = (x0 > q * x1) ? 0 : 1;  
		}
		// x0는 미리 저장해준 기존 x1값에  맞춰 대입해준다.
		// 여기까지의 과정이 기존의
        // tem = x0 - q * x1; x0 = x1; x1 = tem;  작업이다.
		x0 = t; isNx0 = isNt;
    }
    // 서로소인지 확인
    if (d1 == 1){
		// isNx1의 부호를 확인하고 음수면 m으로 빼준다.
        // (기존의 m + x1과 동일한 결과)
		// 양수라면 그냥 x1을 리턴한다.
    	return (isNx1 > 0 ? m - x1 : x1);
    }
    else
    	return 0;
}

/*
 * Miller-Rabin Primality Testing against small sets of bases
 *
 * if n < 2^64,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
 *
 * if n < 3317044064679887385961981,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, and 41.
 */
static const uint64_t a[BASELEN] = {2,3,5,7,11,13,17,19,23,29,31,37};

/*
 * miller_rabin() - Miller-Rabin Primality Test (deterministic version)
 *
 * n > 3, an odd integer to be tested for primality
 * It returns 1 if n is prime, 0 otherwise.
 */
static int miller_rabin(uint64_t n)
{
	// 변수 설정
	uint64_t q = n - 1, tem, p, a_val;
	
	// 2보다 작거나, 2 제외 짝수는 소수 아님
	if (n < 2 || (n != 2 && n % 2 == 0)) return COMPOSITE;
	
	// q 구하기
	while (q % 2 == 0) q /= 2;
	
	// a값들만큼 반복
	for (int i = 0; i < BASELEN; i++){
		// a값이 n과 같다면 소수
		if((a_val = a[i]) == n) return PRIME;
	
		tem = q;
		
		// a^q 가 1이라면 다음 반복문 확인
		if ((p = mod_pow(a_val, tem, n)) == 1) continue;		
		
		// 0 ~ k - 1까지 p가 n - 1인지 확인
		while(tem != n - 1 && p != n - 1){
			p = mod_mul(p, p, n);
			tem *= 2;
		} 

		// tem == n - 1 이라서 끝났다면 조건문
		if (p != n - 1){
			return COMPOSITE;
		}
	}
	
	// 모두 문제 없이 끝났다면 소수
	return PRIME;
}

/*
 * mRSA_generate_key() - generates mini RSA keys e, d and n
 *
 * Carmichael's totient function Lambda(n) is used.

공개키 PU{e, n} 과, 개인키 PR{d, n} 을 구하는 함수이다. 
먼저 두 소수 p와 q의 곱으로 n을 만들기 위해서 p와 q를 랜덤으로 뽑는다. 
이때 n은 무조건 2^63 <= n < 2^64 의 조건을 만족해야 하기에 
여러번 뽑아보면서 조건을 만족하는지 체크해야 한다.

안전하게 랜덤으로 뽑기 위해서 32비트 정수를 리턴해주는 arc4random(); 함수를 이용해준다.

각 p와 q를 뽑아서 두 수의 맨 왼쪽과 오른쪽 비트를 1로 바꾸는데 
이는 해당 수를 홀수로 만들어주는 것의 의미하고 그 편이 유리하다. 
이유는 2가 아닌 짝수는 소수가 아니기 때문이다. 
두번째로 그냥 p와 q를 구하고 계속 소수인지 판정해보고 
소수이면 곱해봤을 때 해당 수가 2^63 이상인지를 판단해야 하는데, 
맨 왼쪽 비트를 1로 바꾸지 않고 계산을 했을 때 그 속도가 굉장히 느렸다. 
p와 q는 반드시 32비트일 필요는 없지만, 
둘 다 32비트로 했을 때 조건에 맞는 p와 q를 찾는 속도가 비약적으로 빨라짐을 확인할 수 있었다.

따라서 두 수의 맨 왼쪽 비트를 1로 만들었다. 
더군다나 p와 q가 비슷한 길이일수록 안정성이 높아진다고 하니 
여러모로 이득이 많을 거라고 판단했다.
32비트 정수는 최대 2^32 - 1이고 (2^32 – 1) * (2^32 – 1) = 2^64 – 1이니 
p*q는 항상 2^64 보다 작아서 오버플로를 걱정할 필요도 없다.

이제 값을 계산해주고 2^63 이상이면 해당 조건을 넘어가 준다.

이후에 n값은 계산해준 s값으로 넣고, 카마이클 함수를 통해서 𝜆(𝑛)를 구해준다.
이는 p – 1과 q – 1의 최소공배수이다.
이후 1 < e < 𝜆(𝑛) 조건에 맞고, gcd(e, 𝜆(𝑛)) == 1 즉 서로소를 만족하는 e를 찾기 위해서 
2 부터 𝜆(𝑛)까지 반복문을 돌려준다. 
e를 찾았다면 반복문을 멈추고 e의 모듈러 𝜆(𝑛)에 대한 역원인 d를 구해주는 것으로
e, d, n값을 모두 구해줄 수 있다.

이렇게 구해진 (e,n)은 공개키로 암호화에 쓰이고 (d,n)은 개인키로 복호화에 쓰인다.
(반대도 가능) 

 */
void mRSA_generate_key(uint64_t *e, uint64_t *d, uint64_t *n)
{
	// 변수 설
	uint64_t p, q, s, l;

	while (1){
		// p 랜덤으로 뽑아서 홀수 + 32비트로 변경
		p = arc4random(); p |= 0x80000001;

		// p는 소수인가?
		if (miller_rabin(p) != PRIME) continue;
		
		// q 랜덤으로 뽑아서 홀수 + 32비트로 변경
		q = arc4random(); q |= 0x80000001;

		// q는 소수인가?
		if (miller_rabin(q) != PRIME) continue;

		// n값 구하기
		s = p * q;

		// n이 2^63보다 작은가?
		if (s < 0x8000000000000000) continue;
		
		break;
	}

	// n에 복사
	*n = s;

	// 카마이클 함수
	l = ((p-1)*(q-1)) / gcd(p-1,q-1);
	
	// e 찾기
	for (int i = 2; i < l; i++){
		if (gcd(i,l) == 1){
			*e = i;
			break;
		}
	}

	// 모듈러 역원으로 d 찾기
	*d = mul_inv(*e, l);
}

/*
 * mRSA_cipher() - compute m^k mod n
 *
 * If data >= n then returns 1 (error), otherwise 0 (success).

암호화와 복호화인 m^k mod n을 해주는 함수를 구현한다.
m은 무조건 n보정다 작아야 하기에 만약 더 크다면 
난수로 뽑은 m이 너무 크다며 1을 리턴해주고, 
그렇지 않다면 mod_pow로 m^k mod n를 계산하고 0을 리턴해준다.

 */
int mRSA_cipher(uint64_t *m, uint64_t k, uint64_t n)
{
	if (*m >= n) return 1;
	*m = mod_pow(*m, k, n);
	return 0;
}









