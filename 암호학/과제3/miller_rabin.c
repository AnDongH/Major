/*
 * Copyright(c) 2020-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 자는 이 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */
#include "miller_rabin.h"
#include <math.h>
/*
 * mod_add() - computes a+b mod m
 * a와 b가 m보다 작다는 가정하에서 a+b >= m이면 결과에서 m을 빼줘야 하므로
 * 오버플로가 발생하지 않도록 a-(m-b)를 계산하고, 그렇지 않으면 그냥 a+b를 계산하면 된다.
 * a+b >= m을 검사하는 과정에서 오버플로가 발생할 수 있으므로 a >= m-b를 검사한다.

오버플로를 고려해서, a + b의 m에 대한 모듈러 연산을 해주는 함수이다.
먼저 임시 변수로 a와 b의 모듈러 연산 값을 받아두고, 해당 값으로 연산을 진행해 주었다.
이때 r1 + r2  > m 이면 m을 더해주어야 하므로 분기를 나누어야 하는데, 
조건식에서 오버플로가 날 가능성이 있으므로 
r1 < (m – r2)와 r1 – (m – r2) 처럼 연산의 순서를 바꾸어서 오버플로를 막아준다.
 */

uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m)
{
	// 임시 변수 선언 % m
	uint64_t r1 = a % m;
	uint64_t r2 = b % m;
	
	// 오버 플로 나지 않도록 조건
	return r1 < (m - r2) ? r1 + r2 : r1 - (m - r2);
}

/*
 * mod_sub() - computes a-b mod m
 * 만일 a < b이면 결과가 음수가 되므로 m을 더해서 양수로 만든다.

위 mod_add처럼 분기를 나누어서 모듈러 연산을 리턴해주면 된다.
뺄셈에서는 오버플로가 날 가능성은 딱히 없으므로 (a – b + m)과, (a - b) 를 그대로
리턴해준다.

 */
uint64_t mod_sub(uint64_t a, uint64_t b, uint64_t m)
{
	// a가 작으면 m 더해주기
	if (a < b) return (a-b+m) % m;
	// 아니면 그냥 리턴
	else return (a-b) % m;	
}

/*
 * mod_mul() - computes a*b mod m
 * a*b에서 오버플로가 발생할 수 있기 때문에 덧셈을 사용하여 빠르게 계산할 수 있는
 * "double addition" 알고리즘을 사용한다. 그 알고리즘은 다음과 같다.
 *     r = 0;
 *     while (b > 0) {
 *         if (b & 1)
 *             r = mod_add(r, a, m);
 *         b = b >> 1;
 *         a = mod_add(a, a, m);
 *     }

이전에 과제 1에서 했던 다항식 곱셈 코드와 매우 유사하다. 
하지만 조금 다르다.
해당 코드는 a에 계속 a를 더해주고(계속 곱하기 2) b를 1비트씩 왼쪽으로 밀어서,
1이라면 r에 더해주고 있는데, 이런 의미이다.
기본적으로 수를 이진수로 표현하면 1001011 이런식이다. 
그리고 이는 2^6 + 0 + 0 + 2^3 + 0 + 2^1 + 2^0 과 같다. 
그렇다면 a에 계속 2를 곱해주는 것이 이해가 간다. 
오른쪽부터 1비트씩 돌 때마다 a*(2^0), a*(2^1), a*(2^2) ……. 로 진행이 되기 때문이다.
a * b라는 것은 결국 a를 b번 더한 것이다. 
(물론 2 * 0 등이 설명이 되지 않는 등 실제 수학적 의미와는 조금 다르다.) 
따라서 a에는 계속 누적으로 2를 곱해주고, b를 1비트씩 오른쪽으로 밀어서 
만약 해당 비트가 1이라면 누적해 놓은 a를 r에 더해주면 되는 것이다.

위에 적어둔 이진수를 예로 들면
2^0 일 때 a*(2^0) 를 더해주고 ……..  2^6 일 때 a*(2^6) 을 더해주는 것이다. 
이렇게 한다면 a를 b번 더해주는 것이 가능하다.

마지막에 모듈러 m을 해주지 않는 이유는 덧셈 계산중에 전부 mod m이 되었기 때문이다.


 */
uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m)
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
 * a^b에서 오버플로가 발생할 수 있기 때문에 곱셈을 사용하여 빠르게 계산할 수 있는
 * "square multiplication" 알고리즘을 사용한다. 그 알고리즘은 다음과 같다.
 *     r = 1;
 *     while (b > 0) {
 *         if (b & 1)
 *             r = mod_mul(r, a, m);
 *         b = b >> 1;
 *         a = mod_mul(a, a, m);
 *     }

위와 같은 논리로, a^b는 a를 b번 곱해준 것이기 때문에 
mod_add 대신 mod_mul을 넣어주면 mod_pow가 계산되어, a^b를 리턴해준다.

 */
uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m)
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
 * Miller-Rabin Primality Testing against small sets of bases
 *
 * if n < 2^64,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
 *
 * if n < 3,317,044,064,679,887,385,961,981,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, and 41.
 */
const uint64_t a[BASELEN] = {2,3,5,7,11,13,17,19,23,29,31,37};

/*
 * miller_rabin() - Miller-Rabin Primality Test (deterministic version)
 *
 * n > 3, an odd integer to be tested for primality
 * It returns PRIME if n is prime, COMPOSITE otherwise.

강의 노트에 있는 슈도 코드와는 조금 다르게 짜 보았다. 
miller_rabin알고리즘은 결국 페르마의 정리를 이용하는 방법인데,
페르마 정리는 a^(p-1) ≡ 1 (mod p) (만약 p가 소수라면) 이다.
소수 판정을 할 수는 기본적으로 홀수이다. 2를 제외한 짝수는 모두 소수이기 때문이다.
따라서 n – 1 은 2^k * q 로 나타낼 수 있다. 
여기서 페르마의 정리를 이용해보면 이런 전개식을 얻을 수 있다.
(2^k * q) = n - 1
a^(2^k * q) == 1 mod n
a^(2^k * q) - 1 == 0 mod n
a^(2^k * q) - 1 =  (a^(2^(k-1) * q) + 1)(a^(2^(k-2) * q) + 1) ......(a^q + 1)(a^q - 1)
이 곱들 중 하나라도 0이면 소수일 가능성이 있다는 것이다.
따라서 또 저 전개식이 0이 되기 위해서는 마지막 곱셈에서 a^q가 1이여야 하는 것을 제외하면 
나머지는 -1 이 되어야 한다. 적어도 하나는 이를 만족해야 한다. 
이때 mod n 계산이므로 n을 더해 양수로 만들면 n-1이나 1이 나와줘야 한다. 
그렇다면 소수일지도 모른다는 결론이 나올 수 있다.

이때 deterministic version이니 정해진 a로 판정을 한다면 정확한 소수 판정이
가능하다.

코드는 이러하다.
먼저 2보다 작거나, 2가 아닌 짝수는 전부 소수가 아니라고 리턴한다.
그리고 q값을 구하기 위해 q = n – 1 로 두고 홀수가 될 때까지 2로 나누어 준다.
 a 배열의 길이만큼 판정을 반복하고
만약 n이 해당 배열 안의 숫자라면 바로 소수라고 판정한다.
먼저 a^q 가 1인지를 판단하고 참이면 다음 a값을 확인해준다.

이제 tem != n – 1 && p != n – 1일 때 까지 
p = mod_mul(p, p, n);와 tem *= 2;를 반복해주는데, 
이건 tem의 경우 강의 노트 상 0 ~ k – 1 까지의 반복을 나타내기 위함이고 
p의 경우 a^(2^j * q)  (단 j = 0 ~ k - 1)가 n - 1인지 확인해주는 부분이다. 
2의 지수승이 계속 a의 지수로 올라가니 mod_mul을 이용해서 자기 자신을 곱해주면 만들 수 있다.
만약 tem == n – 1이 된다면 반복문을 모두 돌았음을 의미하고
p == n – 1이 된다면 더 이상의 반복문은 의미가 없음을 의미한다.
해당 반복문이 끝났다면 tem == n - 1조건으로 끝났음을 대비해서

조건문을 넣어주고, 아니라면 다음 a값을 확인해주는 것이다.
이제 모든 a값을 확인했을 때까지 프로그램이 끝나지 않았다면, 
해당 n은 소수임을 나타내므로 PRIME을 리턴해준다.


 */
int miller_rabin(uint64_t n)
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













