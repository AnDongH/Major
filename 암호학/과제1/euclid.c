/*
 * Copyright(c) 2020-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 자는 이 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */
#include "euclid.h"

/*
 * gcd() - Euclidean algorithm
 *
 * 유클리드 알고리즘 gcd(a,b) = gcd(b,a mod b)를 사용하여 최대공약수를 계산한다.
 * 만일 a가 0이면 b가 최대공약수가 된다. 그 반대도 마찬가지이다.
 * a, b가 모두 음이 아닌 정수라고 가정한다.
 * 재귀함수 호출을 사용하지 말고 while 루프를 사용하여 구현하는 것이 빠르고 좋다.
 */
int gcd(int a, int b)
{
 	 /* 
		 gcd(a, b) = gcd(b, a % b) 와 같음을 이용한 방식이다.
   		 재귀 호출은 일반적으로 스택에 계속해서 메모리를 쌓아가고
   		 다시 복귀주소로 리턴해주는 과정을 가지기에 특별한 경우가 아니라면
   		 반복문이 더 높은 성능을 발휘한다.

  		 매 반복마다 a 에는 b를 넣고, b에는 a % b를 넣는 과정을 진행한다.
  	*/ 	
	while(b != 0){
		int tem = b;
		b = a % b;
		a = tem;
	}
	return a;
}

/*
 * xgcd() - Extended Euclidean algorithm
*
 * 확장유클리드 알고리즘은 두 수의 최대공약수 gcd(a,b) = ax + by 식을
 * 만족하는 x와 y를 계산하는 알고리즘이다. 강의노트를 참조하여 구현한다.
 * a, b가 모두 음이 아닌 정수라고 가정한다.
 */
int xgcd(int a, int b, int *x, int *y)
{
	/*
		gcd(a,b) = d = ax + by의 식을 만족한다고 했을 때,
		gcd(a,b) = gcd(b,a%b) = gcd(a%b,b%(a%b)) 과정으로 나아가며
		이때 각각의 인자값들을 d0, d1, d2, d3 ..... 로 가정할 수 있다.
		따라서 이걸 ax + by의 식으로 나타내면 이러하다.
		d0 = a = ax0 + by0
		d1 = b = ax1 + by1
		d2 = d0 - (d0/d1)d1 = ax2 + by2 (d의 연산은 d0 mod d1)
		d3 = d1 - (d1/d2)d2 = ax3 + by3 (d의 연산은 d1 mod d2)

		위 과정을 반복했을 때 일반식이 나온다.
		몫 = qi = di-1 div d1
		나머지 = di+1 = (di-1) - qi * di
		
		이 과정에서 x, y에 관해서도 똑같이 적용되는데,
		xi+1 = (xi-1) - qi * xi
		yi+1 = (yi-1) - qi * yi
		의 과정을 반복한다.

		이 과정을 끝까지 했을 때의 값 즉
		dk + 1 = 0 -> gcd(a,b) = dk = axk + byk일 때의 x와 y의 값을 구해주면
      	된다.
    */

	// 변수 초기화
	int x0 = 1, y0 = 0;
	int x1 = 0, y1 = 1;  
	int q, r, tem_x, tem_y;              

	while (b != 0){
		// 몫, 나머지 계산
		// di+1 = (di-1) - qi * di
		q = a / b;
		r = a % b;
		
		// xi+1 = (xi-1) - qi * xi
		// yi+1 = (yi-1) - qi * yi
		tem_x = x1;
		tem_y = y1;

		x1 = x0 - q * x1;
		y1 = y0 - q * y1;

		x0 = tem_x;
		y0 = tem_y;

		// d값 변경
		a = b;
		b = r;
	}
	// xk, yk 대입
	*x = x0;
	*y = y0;

	// gcd(a,b) 값 리턴
	return a; 

}
/*
 * mul_inv() - computes multiplicative inverse a^-1 mod m
 *
 * 모듈로 m에서 a의 곱의 역인 a^-1 mod m을 구한다.
 * 만일 역이 존재하지 않으면 0을 리턴한다.
 * 확장유클리드 알고리즘을 변형하여 구현한다. 강의노트를 참조한다.
 */
int mul_inv(int a, int m)
{   

    /*
		만약 a,b가 서로소라면 둘의 최대공약수는 1이다.
		따라서 gcd(a,b) = 1
		
		만약 dk = 1
		1 = axk + byk
		1 mod b = axk + byk mod b = 1
		우리는 a의 xk에만 관심이 있기 때문에 b를 버리자
		따라서 axk mod b = 1
		이므로 xk = a^-1 mod b 이다.
		
		즉 a, m이 서로소임을 확인하고,
	    xk의 값을 구해준다.
	*/

	// 변수 초기화
	int d0 = a, d1 = m;
	int x0 = 1, x1 = 0, q, tem;

	// d1 > 1까지 진행하는 이유는
	// 마지막 반복을 돌고 d1을 확인해봤을 때
	// d1 = 1이라면 둘의 공약수가 1밖에 없다는 소리이고
	// d1 = 0이라면 둘의 공약수가 1이 아닌 무언가가 존재한다는 소리
	while (d1 > 1){
		// 몫 계산
		q = d0 / d1;

		// di+1 = (di-1) - qi * di
		// xi+1 = (xi-1) - q1 * xi
		tem = d0 - q * d1; d0 = d1; d1 = tem;
		tem = x0 - q * x1; x0 = x1; x1 = tem;
	}
	
	// 서로소인지 확인
	if (d1 == 1){
		// x는 음수와 양수가 왔다갔다 거린다.
		// 만약 x가 음수라면 mod m 계산이니 m을 더해줘도 없는 취급이 되므로
		// 동일한 값의 연산이나 마찬가지이다. 따라서 m을 더해 양수로 만들어준다.
		return (x1 > 0 ? x1 : x1 + m);
	}
	else
		return 0;
}

/*
 * umul_inv() - computes multiplicative inverse a^-1 mod m
 *
 * 입력이 unsigned 64 비트 정수일 때 모듈로 m에서 a의 곱의 역인 a^-1 mod m을 구한다.
 * 만일 역이 존재하지 않으면 0을 리턴한다. 확장유클리드 알고리즘을 변형하여 구현한다.
 * 입출력 모두가 unsigned 64 비트 정수임에 주의한다.
 */
uint64_t umul_inv(uint64_t a, uint64_t m)
{   
	/*
		64bit unsigned int의 특징은 음수를 표현하지 않는다.
		근데 중요한 점은 이미 mul_inv에서 언급했지만
		x의 값은 계산 도중에 음수와 양수값을 둘 다 가질 수 있다는 점이다.

		참고로 d값은 그냥 mod 연산이기에 절대 음수일 수 없다.

		그럼 가장 간단하고 직관적으로 이해할 수 있는 방식을 생각해본다면
		그냥 우리가 따로 음수 표현을 가지도록 만들어주고 계산해주면 된다.

		int inNx0, inNx1, isNt는 0, 1의 값만 가지는 변수로 각각
		uint64_t의 x0, x1, t 값의 음수 표현을 도와주는 변수다.
		0이면 양수, 1이면 해당 변수가 음수임을 나타낸다.

		기본적으로 알고리즘의 진행 방식은 mul_inv와 같다. 딱 하나 다른점은
		x값의 계산에서 isNx0, isNx1 값을 검사해 계산 방식을 다르게 해주면 된다.
	*/


	// 변수 설정	
	uint64_t d0 = a, d1 = m;
    uint64_t x0 = 1, x1 = 0, q, tem, t;
    int isNx0 = 0, isNx1 = 0, isNt = 0;
   
	// d1 > 1 동안  
    while (d1 > 1){
		// d 계산은 동일
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
 * gf16_mul(a, b) - a * b mod x^16+x^5+x^3+x+1
 *
 * 15차식 다항식 a와 b를 곱하고 결과를 16차식 x^16+x^5+x^3+x+1로 나눈 나머지를 계산한다.
 * x^16 = x^5+x^3+x+1 (mod x^16+x^5+x^3+x+1) 특성을 이용한다.
 */
uint16_t gf16_mul(uint16_t a, uint16_t b)
{
	/*
		GF(2^n) 내에서의 연산은 mod 2 즉 XOR 연산과 같다.
		우리는 곱셈을 구해야 하므로 shift 연산과 XOR 연산을
		적절히 활용해야 한다.
		

		15차식의 곱셈의 결과를 16차식으로 나눈 나머지를 계산하고
		위의 특성을 이용해야 한다.

		임의로
		b = x^4 + x + 1 = 10011
		a = x^4 + 1 = 10001
		이라고 해보자. a * b는 이렇게 계산될 것이다.
		a * b = (x^4 + 1) * (x^4 + x + 1)
		= a * x^4 + a * 0 + a * 0 + a * x + a * 1
		이후 값들은 mod 2로 계산되니 XOR로 덧셈을 진행해준다.	
		

		이를 오른쪽부터 잘 보면 a에 계속해서 x를 곱해주는데,
		b의 비트가 0이면 더해주지 않고, 부호가 1이면 더해주는 방식이다.
		
		아래 알고리즘도 이를 이용하고 mod 계산은 a에 x를 곱해줄 때 적용해준다.
		이게 가능한 이유는 mod에 분배 법칙이 허용돼 최종값에 mod를 해줄 필요
		없이 중간 과정에 mod를 해줘도 되기 때문이다.

		또한 위 특성을 적용해서 mod 계산은 x^5 + x^3 + x + 1
		= 0010 1011 = 0x2B를 적용해준다.		
	*/
	
	uint16_t r = 0;
    while (b > 0) {
		// b의 마지막 비트가 1인지 0인지 확인
		// 1이면 a 를 r에 XOR 해준다. 최초값은 상수곱을 해준 경우
		if (b & 1) r = r ^ a;

		// b의 비트를 오른쪽으로 한칸 민다(다음 비트 확인 위해) 
		b = b >> 1;

		// a에 x를 곱해주고(왼쪽으로 1비트 밀기) 
		// 만약 15차항이 존재한다면(15비트 오른쪽으로 민 것이 1이라면)
		// mod x^16+x^5+x^3+x+1 = x^5+x^3+x+1 = 0x2B를 적용해준다.
		a = ((a<<1) ^ ((a>>15) & 1 ? 0x2B : 0));

		// 위 과정을 반복해주면
		// a * x^4 + a * 0 + a * 0 + a * x + a * 1
		// 꼴과 같은 과정이 완성된다.
    }
    return r;
}

/*
 * gf16_pow(a,b) - a^b mod x^16+x^5+x^3+x+1
 *
 * 15차식 다항식 a를 b번 지수승한 결과를 16차식 x^16+x^5+x^3+x+1로 나눈 나머지를 계산한다.
 * gf16_mul()과 "Square Multiplication" 알고리즘을 사용하여 구현한다.
 */
uint16_t gf16_pow(uint16_t a, uint16_t b)
{
	/*
		위의 gf16_mul과, multiplication 알고리즘을 적절히 조합해
		해당 함수를 구현해보자. 단순히 multiplication 알고리즘의
		기본 모형에 곱셈 부분만 gf16_mul을 적용하였다.
		gf16_mul에서 어차피 mod 계산이 되었기에
		따로 계산을 진행하지는 않았다.
	*/	

	uint16_t r = 1;
	while (b > 0) {
		if (b & 1) r = gf16_mul(r, a);
		b = b >> 1;
		a = gf16_mul(a,a);
	}
	return r;
}

/*
 * gf16_inv(a) - a^-1 mod x^16+x^5+x^3+x+1
 *
 * 모둘러 x^16+x^5+x^3+x+1에서 a의 역을 구한다.
 * 역을 구하는 가장 효율적인 방법은 다항식 확장유클리드 알고리즘을 사용하는 것이다.
 * 다만 여기서는 복잡성을 피하기 위해 느리지만 알기 쉬운 지수를 사용하여 구현하였다.
 */
uint16_t gf16_inv(uint16_t a)
{
    return gf16_pow(a, 0xfffe);
}
