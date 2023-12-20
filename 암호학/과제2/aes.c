/*
 * Copyright(c) 2020-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 자는 이 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 * 2018037356 컴퓨터 학부 안동현 수정
 */
#include "aes.h"
#include <string.h>

static const uint8_t sbox[256] = {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const uint8_t isbox[256] = {
  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

static const uint8_t Rcon[11] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

static const uint8_t M[16] = {2, 3, 1, 1, 1, 2, 3, 1, 1, 1, 2, 3, 3, 1, 1, 2};

static const uint8_t IM[16] = {0x0e, 0x0b, 0x0d, 0x09, 0x09, 0x0e, 0x0b, 0x0d, 0x0d, 0x09, 0x0e, 0x0b, 0x0b, 0x0d, 0x09, 0x0e};

// 복호화 전용 roundKey
static uint32_t droundKey[RNDKEYLEN];

/*
이번 과제를 진행하면서 스켈레톤 코드를 제외하고 작성한 추가 함수들의 목록입니다.
KeyExpansion을 위한 SubWord, RotWord가 있고,
Cipher을 위한 AddRoundKey, SubBytes, ShiftRows, MixColumns가 있고,
ShiftRows를 쉽게 도와주기 위한 Shift와
MixColumns를 쉽게 도와주기 위해 이전 과제에서 진행한 GF(2^8)에서의 다항식 계산 함수인
Mul이 있습니다.
*/
uint32_t SubWord(const uint32_t);
uint32_t RotWord(const uint32_t);
void AddRoundKey(uint8_t*, const uint32_t*);
void SubBytes(uint8_t*, int);
void Shift(uint8_t*, int, int, int);
void ShiftRows(uint8_t*, int);
uint8_t Mul(uint8_t, uint8_t);
void MixColumns(uint8_t*, int);

/*
 * Generate an AES key schedule
 */

/*
키 확장의 과정은 AddRoundKey 연산에서 사용될 w 즉 roundKey를 만드는 과정을 의미한다. 연
산을 간단히 설명하자면, 먼저 key는 8비트 정수의 배열이지만 roundKey는 32비트 정수의 배열
임을 인지하고 있자. 즉 key의 4개의 원소를 합친 것이 roundKey 원소 하나라는 것이다.

1번째로 w[0,3]까지는 기존의 키값들을 32비트 정수로 만들어 넣어준다.
리틀 엔디안 방식을 고려해서 보기 편하게 8비트 정수 포인터 타입으로 캐스팅해
서 각각의 p[0], p[1] ….에 넣어줬다. key는 각각의 열이 들어가야 하므로 4*i+(0….3) 을 사용해준
다.

이제 이후로 나오는 w[4……..] 에서는 이전의 값들을 이용해서 만든다. 방금 4개를 만들었으니 40
개만 더 만들면 된다. 

이전에 4개를 만들었으니 i = Nk로 초기화해준다. 그리고 Nb * (Nr + 1) == 44 만큼 반복문을 돌
려준다. 바로 이전 w인 temp = roundKey[i-1]; 를 받아주고, 이 과정에서 i가 Nk로 나누어 떨어
지는 부분은 g연산을 진행해준다.

문서에 맞게 RotWord와 SubWord 함수를 이용해주고 Rcon값과 XOR 연산을 해준다.
만약 key의 비트수가 늘어난다면 else if 문이 돌아간다.
그리고 Nk로 나누어 떨어지지 않는다면 Nk만큼 이전의 w와 temp를 XOR 해준다.

이렇게 한다면 roundKey값을 만드는 건 끝난다. 근데 이때 추가적인 작업이 하나 들어갈 수 있는
데,

Inverse Mix Cols를 통해 특수한 roundKey를 하나 더 만들어서 암호화와 복호화의
과정이 동일하도록 만드는 roundKey를 만들 것이다.

droundKey라는 배열을 만들어서 roundKey를 복사해준 다음, BLOCKLEN * 8비트정수사이즈 만큼
잘라서 Inverse Mix Cols을 해주는 것이다.
memcpy로 잘라서 Inverse Mix Cols를 진행해주고, 다시 memcpy 로droundKey 값을 바뀐 값으로
바꿔주는 연산이다. 이렇게 하면 droundKey의 모든 부분에 연산이 진행되어서 새로운 w가 만들
어진다. 이제 복호화 과정에서는 roundKey대신 droundKey를 사용하고, 연산 순서를 암호화와 같
이 해주면 된다.
*/

// w를 만드는 키 확장 알고리즘
void KeyExpansion(const uint8_t *key, uint32_t *roundKey)
{
	// 변수 선언
	int i = 0;
	uint32_t temp;
	uint8_t *p;

	// w[0,3] 제작
	while (i < Nk){
		p = (uint8_t*)(roundKey+i);
		p[0] = key[4*i+0];
		p[1] = key[4*i+1];
		p[2] = key[4*i+2];
		p[3] = key[4*i+3];		
		i++;
	}
	
	i = Nk; // 4
	// w[4,43] 제작
	// 40번 반복
	while (i < Nb * (Nr+1)){
		// 바로 이전 w
		temp = roundKey[i-1];

		// g연산
		if (i % Nk == 0){
			temp = SubWord(RotWord(temp)) ^ Rcon[i/Nk];
		}
		else if (Nk > 6 && (i % Nk == 4))
			temp = SubWord(temp);

		// Nk이전 것과 temp XOR
		roundKey[i] = roundKey[i-Nk] ^ temp;
		i++;	
	}

	// 복호화용 roundKey제작
	for (int i = 0; i < Nb * (Nr+1); i++){
		droundKey[i] = roundKey[i];
	}

	for (int i = 1; i < Nr; i++){
		uint8_t tem[BLOCKLEN];
		memcpy(tem, droundKey + i*Nb, sizeof(uint8_t) * BLOCKLEN);
		MixColumns(tem, DECRYPT);
		memcpy(droundKey + i*Nb, tem, sizeof(uint8_t) * BLOCKLEN);
	}
}

/*
해당 함수는 KeyExpansion에서 사용되는 함수로 키 확장을 할 때 g연산을 진행하게 되는데 그때
32비트 정수의 각 위치의 8비트 정수들을 sbox에 넣어서 나온 새로운 8비트 정수값으로 다시 32
비트 정수를 구성하는 함수다. 각각을 24, 16, 8, 0비트씩 오른쪽으로 민 것을 0xFF와 and 연산을
통해 8비트값을 얻어내고 그것을 32비트로 캐스팅 한 후에 원래 위치로 다시 왼쪽으로 밀어 or 
연산으로 합쳐주는 과정이다. 리틀 엔디안, 빅 엔디안 방식에 상관 없이 어차피 원래 위치로 돌려
주기만 하면 되기에 보기 편하게 8비트 캐스팅을 통해 4개로 나누어 구현하지 않고, 32비트 정수
값에 한번에 접근해서 구현하였다. 8비트 캐스팅을 하면 0, 1, 2, 3 위치에 한번씩 메모리 접근을
해야해서 메모리 접근이 많이 필요하지만 해당 방식으로 구현하면 32비트에 한번에 접근해서 구
현하는 것이 가능하다.
*/

// 32비트 정수를 8비트로 잘라 sbox에 넣어 반환되는 값으로 다시 32비트 정수 구성
uint32_t SubWord(const uint32_t rw){
	return ((uint32_t)sbox[(rw >> 24) & 0xFF] << 24) |
		   ((uint32_t)sbox[(rw >> 16) & 0xFF] << 16) |
		   ((uint32_t)sbox[(rw >> 8) & 0xFF] << 8) |
		   ((uint32_t)sbox[rw & 0xFF]);
}

/*
들어온 32비트 정수를 LRot 해주는 함수이다. 하지만 실제 코드 구현은 RRot처럼 보이는데, 그
이유는 현재 인텔 cpu를 사용중이기에 리틀 엔디안 방식이기 때문이다. 들어온 32비트 정수를 8
비트 왼쪽으로 민 것과, 24비트 오른쪽으로 민 것을 or 연산을 통해 합쳐주었다.
*/

// 32비트 정수의 첫번째 8비트를 뒤로 돌리기
// 리틀 엔디안이라서 코드상 보이는건 반대
uint32_t RotWord(const uint32_t w){
	return (w >> 8) | (w << 24); 
}

/*
 * AES cipher function
 * If mode is nonzero, then do encryption, otherwise do decryption.
 */

/*
암호화든, 복호화든 연산을 위한 roundKey와, 반복문의 진행 방
향만 다를 뿐 연산의 순서는 동일하게 진행하였음을 볼 수 있다.
라운드에 들어가기 전
AddRoundKey
1 ~ 9라운드 까지는
SubBytes -> ShiftRows -> MixColumns -> AddRoundKey
10라운드는
SubBytes -> ShiftRows -> AddRoundKey
과정을 사용한다.

이 과정에서 w[0,3], w[4,7], ……. w[40,43]을 이용하기 위해 uint32_t temKey[Nb];를 선언하여
roundKey의 특정 위치를 복사해준다. 첫 w[0,3]은 주소상 roundKey부터 Nb개의 32비트 정수를
복사, 나머지는 roundKey + 라운드 * Nb 부터 Nb개의 32비트 정수를 복사해준다.
그렇게 나온 temKey를 w로서 사용해서 AddRoundKey에 넣어준다.
물론 복호화는 droundKey를 사용해주어야 한다.
*/

// 암호화 알고리즘
void Cipher(uint8_t *state, const uint32_t *roundKey, int mode)
{
	// w[x,y] 표현해줄 임시 배열
	uint32_t temKey[Nb];

	// 암호화
	if (mode == ENCRYPT){
		// w[0,3]
		memcpy(temKey, roundKey, sizeof(uint32_t) * Nb);
      	AddRoundKey(state, temKey);
  
      	for (int i = 1; i < Nr; i++){
        	SubBytes(state, mode);
        	ShiftRows(state, mode);
        	MixColumns(state, mode);
          	
			// w[4,7](1라운드) ~ w[36, 39](9라운드)
          	memcpy(temKey, roundKey + i*Nb, sizeof(uint32_t) * Nb);
          	AddRoundKey(state, temKey);
      	}
      
      	SubBytes(state, mode);
      	ShiftRows(state, mode);
      	
		// w[40,43](10라운드)
      	memcpy(temKey, roundKey + Nr*Nb, sizeof(uint32_t) * Nb);
      	AddRoundKey(state, temKey);

	}
	// 복호화
	else if (mode == DECRYPT){
		// dw[40,43](10라운드)
		memcpy(temKey, droundKey + Nr*Nb, sizeof(uint32_t) * Nb);
		AddRoundKey(state, temKey);

		for (int i = Nr - 1; i > 0; i--){
			SubBytes(state, mode);	
			ShiftRows(state, mode);
			MixColumns(state, mode);

			// dw[36,39](9라운드) ~ dw[4,7](1라운드)
			memcpy(temKey, droundKey + i*Nb, sizeof(uint32_t) * Nb);
			AddRoundKey(state, temKey);
 		}

		SubBytes(state, mode);
 		ShiftRows(state, mode);
		
		// dw[0,3]
		memcpy(temKey, droundKey, sizeof(uint32_t) * Nb);
 		AddRoundKey(state, temKey);

	}
}

/*
현재 state의 열 부분과, roundKey를 XOR 연산을 해주는 함수이다. roundKey에 대한 인덱스 접근
을 편하게 하기 위해, for문을 Nb만큼만 돌리고, state는 4줄을 써주었다. 또한 보기 좋게 작성함
과 동시에 32비트 정수를 shift하고, & 연산을 해주는 것이 보기에 번거로워 8비트 정수의 포인터
로 캐스팅해서 받은 후 각각에 접근해서 XOR 연산을 진행했다.
State를 보면 i*Nb 방식으로 인덱스에 접근하고 있는데, 이를 이용하면 매 반복마다.
0,1,2,3 | 4,5,6,7 | 8,9,10,11 이런식으로 접근이 가능하다.
*/

// 라운드 키를 XOR 연산을 사용하여 state에 더한다.
void AddRoundKey(uint8_t *state, const uint32_t *roundKey){
	uint8_t *p;
	for (int i = 0; i < Nb; i++){
		p = (uint8_t*)(roundKey+i);
		state[0 + i*Nb] = state[0 + i*Nb] ^ p[0];
		state[1 + i*Nb] = state[1 + i*Nb] ^ p[1];
		state[2 + i*Nb] = state[2 + i*Nb] ^ p[2];
		state[3 + i*Nb] = state[3 + i*Nb] ^ p[3];
	}	
}

/*
단순히 암호화일 경우 sbox에, 복호화일 경우 isbox에 접근해서 해당 값으로 바꿔주는 함수이다.
state의 길이는 BLOCKLEN이므로 해당 길이만큼 반복을 시켜줬다.
*/

// mode에 따라 순방향 또는 역방향으로 바이트를 치환한다.
void SubBytes(uint8_t *state, int mode){

	// 암호화
	if (mode == ENCRYPT){
		for (int i = 0; i < BLOCKLEN; i++){
			state[i] = sbox[state[i]];
		}	
	}
	// 복호화
	else if (mode == DECRYPT){
		for (int i = 0; i < BLOCKLEN; i++){
			state[i] = isbox[state[i]];
		}
	}	

}

/*
ShiftRows 연산을 편하게 해주기 위해 만든 함수이다. 각 인자는 이를 의미한다.
state -> 기존 state
start -> Shift작업을 해줄 행의 위치
term -> 각 원소의 간격은 얼마인가?
n -> 몇 개의 위치를 변경할 것인가?

원리는 앞쪽의 옮길 원소들을 임시 배열에 저장해두고, 해당 원소를 제외한 나머지를 앞으로 끌
고와준다. 그리고 나머지 뒤쪽에 임시배열에 저장해 두었던 걸 넣어주면 된다.
*/

// start->시작 위치(행), term->각 원소 간격, n->몇개 변경?
void Shift(uint8_t *state, int start, int term, int n){
	uint8_t tem[n];
	
	// 변경할 원소 담기
	for (int i = 0; i < n; i++)
		tem[i] = state[start + i*term];

	// 나머지 원소들 앞으로 끌고오기
	for (int i = 0; i < term - n; i++)
		state[start + i*term] = state[start + (i+n) * term];
 
	// 임시배열에 담아뒀던 것들 뒤에다가 넣기
	for (int i = 0; i < n; i++)
		state[start + (term - n + i) * term] = tem[i];	
}

// mode에 따라 순방향 또는 역방향으로 바이트의 위치를 변경한다.
void ShiftRows(uint8_t *state, int mode){
	// 암호화
	if (mode == ENCRYPT){
		// 1행 앞쪽 1개 원소 뒤로
		Shift(state, 1, Nb, 1);
		// 2행 앞쪽 2개 원소 뒤로
		Shift(state, 2, Nb, 2);
		// 3행 앞쪽 3개 원소 뒤로
		Shift(state, 3, Nb, 3);
	}
	// 복호화
	else if (mode == DECRYPT){
		// 1행 앞쪽 3개 원소 뒤로
		Shift(state, 1, Nb, 3);
		// 2행 앞쪽 2개 원소 뒤로
		Shift(state, 2, Nb, 2);
		// 3행 앞쪽 1개 원소 뒤로
		Shift(state, 3, Nb, 1);
	}
}

/*
MixColumns의 기약 다항식 x^8 + x^4 + 
x^3 + x + 1을 사용한 GF(2^8)에서 행렬 곱셈에서 이용된다.
*/

// GF(2^8) 에서의 다항식 곱셈 알고리즘
uint8_t Mul(uint8_t a, uint8_t b){
	uint8_t r = 0;
	while (b > 0){
		if (b & 1) r = r^a;
		b = b >> 1;
		a = XTIME(a);
	}
	return r;
}

/*
문서의 연산에 맞게 다항식 곱셈을 이용하면 쉽게 해결이 가능하다.
코드는 먼저 uint8_t tem[BLOCKLEN];으로 임시 배열을 만들어주고
memcpy(tem, state, sizeof(uint8_t)*BLOCKLEN); 로 기존 state를 복사해준다. 그리고
아까처럼 Nb만큼 반복을 통해 state의 0,1,2,3 | 4,5,6,7 | 8,9,10,11 에 접근해서 연산을 진행해주면 된다
*/
// 기약 다항식 x^8 + x^4 + x^3 + x + 1을 사용한 GF(2^8)에서 행렬 곱셈을
// 수행한다. mode가 DECRYPT면 역행렬을 곱한다.
void MixColumns(uint8_t *state, int mode){
	// state대신 연산에 사용될 tem
	uint8_t tem[BLOCKLEN];
	// 복사
	memcpy(tem, state, sizeof(uint8_t)*BLOCKLEN);

	// 암호화 (행렬 M 이용)
	if (mode == ENCRYPT){
		for (int i = 0; i < Nb; i++){
			state[0+i*4] = Mul(tem[0+i*4], M[0]) ^ 
						   Mul(tem[1+i*4], M[1]) ^
						   tem[2+i*4] ^
						   tem[3+i*4];

			state[1+i*4] = tem[0+i*4] ^
						   Mul(tem[1+i*4], M[5]) ^
						   Mul(tem[2+i*4], M[6]) ^
						   tem[3+i*4];

			state[2+i*4] = tem[0+i*4] ^
						   tem[1+i*4] ^
						   Mul(tem[2+i*4], M[10]) ^
						   Mul(tem[3+i*4], M[11]);

			state[3+i*4] = Mul(tem[0+i*4], M[12]) ^
						   tem[1+i*4] ^
						   tem[2+i*4] ^
						   Mul(tem[3+i*4], M[15]);	
		}
	}
	// 복호화 (역행렬 IM 이용)
	else if (mode == DECRYPT){
		for (int i = 0; i < Nb; i++){
			state[0+i*4] = Mul(tem[0+i*4], IM[0]) ^
						   Mul(tem[1+i*4], IM[1]) ^
						   Mul(tem[2+i*4], IM[2]) ^
						   Mul(tem[3+i*4], IM[3]);

			state[1+i*4] = Mul(tem[0+i*4], IM[4]) ^
						   Mul(tem[1+i*4], IM[5]) ^
						   Mul(tem[2+i*4], IM[6]) ^
						   Mul(tem[3+i*4], IM[7]);

			state[2+i*4] = Mul(tem[0+i*4], IM[8]) ^
						   Mul(tem[1+i*4], IM[9]) ^
						   Mul(tem[2+i*4], IM[10]) ^
						   Mul(tem[3+i*4], IM[11]);

			state[3+i*4] = Mul(tem[0+i*4], IM[12]) ^
						   Mul(tem[1+i*4], IM[13]) ^
						   Mul(tem[2+i*4], IM[14]) ^
						   Mul(tem[3+i*4], IM[15]);
		}
	}
}








