// Coding Environment : VSCODE
// 201514747 이성주

#include <stdio.h> // perror과 exit 를 위한 헤더 
#include <stdlib.h> // argc, argv , atoi 등을 쓰기 위한 헤더 
#include <iostream>
#include <fstream> // c++ 파일 입출력을 위한 헤더 
#include <cstring> // trace 파일 가운데 부분 추출을 위한 헤더 
using namespace std;

int BinaryCnt(int n, int cnt) // 2의 몇승인지 구하는 재귀함수 
{
    if ((n / 2) != 0) {
        n = n / 2;
        cnt++;
        return BinaryCnt(n, cnt); // 0이 되기 전까지 재귀  
    }
    return cnt; // 최종적으로 몇승인지 리턴 
}

int main(int argc, char** argv)
{
    unsigned long long pickIndex = 0xffffffff, total = 0, hit = 0; //bs=비교를위한 블락크기 
    double hitrate; // 소수로 나타낸 히트 율 
    //tagCnt , indexCnt, offsetCnt =>각자의 갯수 // index 추출한 인덱스.  
    int tagCnt, indexCnt, offsetCnt, index, tagbit, cnt = 0;

    //직접적인 그 주소를 넣어주기 위한 캐시 배열  
    unsigned long long** cache1 = new unsigned long long* [100000]; // cache 크기를 크게잡기 위한 동적할당 
    for (int i = 0; i < 100000; i++) cache1[i] = new unsigned long long[8];
    // 8은 associative 최대 크기 ( 1, 2, 4, 8만 과제에 따르면 가능 ) 

    // LRU를위한 두번 째 캐시  ( totalaccess 를 업데이트 하기위한 캐시 배열 )  
    unsigned long long** cache2 = new unsigned long long* [100000]; // cache 크기를 크게잡기 위한 동적할당 
    for (int i = 0; i < 100000; i++) cache2[i] = new unsigned long long[8];
    // 8은 associative 최대 크기 ( 1, 2, 4, 8만 과제에 따르면 가능 ) 

    ifstream file(argv[1]); // argv[1] 은 곧 trace파일 이름이다. 그 파일을 읽어오는것.  

    if (argc < 5) { // 입력은 무조건 5개여야 실행이 되야함. 그보다 적으면 에러. 
        perror("입력부족");
        exit(0);
    }

    else if (argc > 5) { // 입력은 무조건 5개여야 실행이 되야함. 그보다 많으면 에러. 
        perror("입력초과");
        exit(1);
    }

    //캐시크기,블락크기,N-way의 N , atoi는 일단 argv는 char * 이기때문( 정수로 바꿔줌 ) 
    int Cs = atoi(argv[2]), Bs = atoi(argv[3]), Act = atoi(argv[4]);
    // Cs = 캐시사이즈 , Bs = 블락사이즈, Act = associative ( N-way ) 

    for (int i = 0; i < 100000; i++)
        for (int j = 0; j < Act; j++) // 일단 보든 캐시 0으로 초기화 
        {
            cache1[i][j] = 0;
            cache2[i][j] = 0;
        }

    if (file == NULL) {// trace 파일이 존재하지 않는다면. 
        perror("trace file State is NULL ! ");//perror 는 오류 함수 
        exit(0); // 종료함수  
    }
    int bs = Bs; // Block size 비교를 위해 복사 
    while (bs != 0) { // Block Size는 2의 n승일때 n을 뜻하기 때문에 반드시 2의 n승 꼴 이어야 함 
        if (bs & 1) cnt++; // 만약 2진수 개념에서 그 칸이 1이면 cnt 증가 
        bs = bs >> 1; // 이진수 개념으로 봤을때 한칸씩 미룸  
    }
    if (cnt != 1) { // Block Size는 2의 n승일때 n을 뜻하기 때문에 반드시 2의 n승 꼴 이어야 함 
        perror("Block Size 는 반드시 2의 제곱 승");
        exit(2);
    }
    if (!(Act & 1 || Act & 2 || Act & 4 || Act & 8)) {
        perror("과제를 따르면 associativity가 1,2,4,8 만 가능 ");
        exit(3);
    }

    if ((Cs % (Bs * Act)) != 0) { // 과제물에 쓰여 있는 오류내용.
         // 오류의 원인은 cout에 적어놓은 대로 이다. 
        perror("과제에 따르면 cache크기는 block크기 * associative의 배수여야 함.");
        exit(4);
    }
    offsetCnt = BinaryCnt(Bs, 0);
    // Direct-Mapped 인가 N-way 인가를 판별  
    if (Act != 1) indexCnt = BinaryCnt(Cs / Bs, 0) - BinaryCnt(Act, 0);
    else indexCnt = BinaryCnt(Cs / Bs, 0);

    tagCnt = 32 - indexCnt - offsetCnt;// tag = 32 - index -offset 이라는 공식을 이용 
    //과제의 표준 아웃 풋을 이용. 
    cout << "tag: " << tagCnt << " bits" << endl;
    cout << "index: " << indexCnt << " bits" << endl;
    cout << "offset: " << offsetCnt << " bits" << endl;

    pickIndex = pickIndex >> (tagCnt + offsetCnt); // pickIndex가 32비트 1인데, 인덱스 남기고 다 밀어버림
    // 그럼 index 만큼의 크기의 비트들이 다 1인 상태다.  
    // 이것은 이제 밑에서 tagbit의 index를 뽑아내기 위한 비교 연산을 위해 사용된다. 


    while (!file.eof()) { // 파일의 끝에 닿기 전까지. 
        bool  hitcheck = false; //hit 인지 miss 인지를 판단하는 체크변수. 
        char buffer[100]; // 일단 한 줄을 받아오기 위한 변수. 엔터를 기점으로 한줄 받아온다. 
        char temp[100] = { 0 }; // 거기서 가운데 주소를 받아오기위한 공간. 세 인자중 
                             // 정작 필요한 것은 가운데 부분 뿐이라서 가운데 부분 추출을 할 것. 
        int bCnt = 0, comp = -1; // bCnt는 temp 배열에 저장 할 인덱스를 위해 선언
                                // comp는 띄어쓰기를 받을때마다 갱신하기 위함이며, 가운데를 추출하기위함. 
        file.getline(buffer, 100); // 한 줄씩 읽어오는 함수. buffer에 그 한 줄을 저장한다. 
        for (int i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == ' ') comp *= -1; // 띄어쓰기를 발견했을 시 comp가 1로 바뀌며 가운데부분 추출. 
            if (comp == 1) temp[bCnt++] = buffer[i]; //가운데부분 temp에 복사. 
        }
        tagbit = strtoul(temp, NULL, 16); // 복사한 가운데 주소를 16진수로 대체( 문자열이기 때문 ) 

        tagbit = tagbit >> offsetCnt; // offset bit 를 일단 떼어냄 
        index = tagbit & pickIndex;  // 위에서 밀어버린 pickIndex를 가지고 index 추출 
        tagbit = tagbit >> indexCnt; // 여기서 index bit 부분을 빼야 비로소 tag 비트 부분만 남는다. 

        total++; // 토탈 증가 

        /* LRU 부분입니다. 순서가 오래된 캐시나 비어있는 캐시를 갱신합니다 */
        // hit 일 경우  
        for (int i = 0; i < Act; i++) {
            if (cache1[index][i] == tagbit) {
                hit++;  // 같으면 히트 ! 
                hitcheck = true; // hit 인지 miss 인지 여부 확인 
                cache2[index][i] = total; // cache2의 그 자리의 수를 최신으로 업데이트 
                break;
            }
        }
        // hit이 발생하지 않았을때 ( 히트체크가 false 일때 ) 
        if (hitcheck == false) {
            int minNum = 2100000000; //LRU 를 위해 빈 캐시는 갱신을 해주고 다 찼을 때 오래 된  
            int flag = -1;             //캐시를 갱신 하기 위한 최소 값 찾기 
                                     //즉 , 캐시2가 1 0 0 0이면 0은 빈 캐시인데 flag는 2번재에서 멈추게 될 것
                               //그래서 비었을 땐 거길 갱신, 아니면 제일 오래 된 녀석 갱신 
            for (int i = 0; i < Act; i++) {
                if (cache2[index][i] < minNum) {
                    minNum = cache2[index][i]; // 비었거나 오래된 캐시 검색 
                    flag = i;  // 그 때의 오래된 캐시가 저장됨. 
                }
            }
            cache2[index][flag] = total; // 비거나 오래된 캐시는 무조건 있을 것이기 때문에 cache2에 total값갱신 
            cache1[index][flag] = tagbit; // 그 자리에 cache1 즉 주소를 담는 캐시에 주소 값 갱신. 
        }
    }
    hitrate = (double)hit / (double)total;
    cout.precision(2); // 소수점 둘째자리까지 출력. 
    cout << "Result: total access :" << total << ", hit :" << hit << ", hit rate : " << hitrate << endl;//표준출력 
    return 0;
}