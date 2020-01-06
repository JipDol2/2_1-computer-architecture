// Coding Environment : VSCODE
// 201514747 �̼���

#include <stdio.h> // perror�� exit �� ���� ��� 
#include <stdlib.h> // argc, argv , atoi ���� ���� ���� ��� 
#include <iostream>
#include <fstream> // c++ ���� ������� ���� ��� 
#include <cstring> // trace ���� ��� �κ� ������ ���� ��� 
using namespace std;

int BinaryCnt(int n, int cnt) // 2�� ������� ���ϴ� ����Լ� 
{
    if ((n / 2) != 0) {
        n = n / 2;
        cnt++;
        return BinaryCnt(n, cnt); // 0�� �Ǳ� ������ ���  
    }
    return cnt; // ���������� ������� ���� 
}

int main(int argc, char** argv)
{
    unsigned long long pickIndex = 0xffffffff, total = 0, hit = 0; //bs=�񱳸����� ���ũ�� 
    double hitrate; // �Ҽ��� ��Ÿ�� ��Ʈ �� 
    //tagCnt , indexCnt, offsetCnt =>������ ���� // index ������ �ε���.  
    int tagCnt, indexCnt, offsetCnt, index, tagbit, cnt = 0;

    //�������� �� �ּҸ� �־��ֱ� ���� ĳ�� �迭  
    unsigned long long** cache1 = new unsigned long long* [100000]; // cache ũ�⸦ ũ����� ���� �����Ҵ� 
    for (int i = 0; i < 100000; i++) cache1[i] = new unsigned long long[8];
    // 8�� associative �ִ� ũ�� ( 1, 2, 4, 8�� ������ ������ ���� ) 

    // LRU������ �ι� ° ĳ��  ( totalaccess �� ������Ʈ �ϱ����� ĳ�� �迭 )  
    unsigned long long** cache2 = new unsigned long long* [100000]; // cache ũ�⸦ ũ����� ���� �����Ҵ� 
    for (int i = 0; i < 100000; i++) cache2[i] = new unsigned long long[8];
    // 8�� associative �ִ� ũ�� ( 1, 2, 4, 8�� ������ ������ ���� ) 

    ifstream file(argv[1]); // argv[1] �� �� trace���� �̸��̴�. �� ������ �о���°�.  

    if (argc < 5) { // �Է��� ������ 5������ ������ �Ǿ���. �׺��� ������ ����. 
        perror("�Էº���");
        exit(0);
    }

    else if (argc > 5) { // �Է��� ������ 5������ ������ �Ǿ���. �׺��� ������ ����. 
        perror("�Է��ʰ�");
        exit(1);
    }

    //ĳ��ũ��,���ũ��,N-way�� N , atoi�� �ϴ� argv�� char * �̱⶧��( ������ �ٲ��� ) 
    int Cs = atoi(argv[2]), Bs = atoi(argv[3]), Act = atoi(argv[4]);
    // Cs = ĳ�û����� , Bs = ���������, Act = associative ( N-way ) 

    for (int i = 0; i < 100000; i++)
        for (int j = 0; j < Act; j++) // �ϴ� ���� ĳ�� 0���� �ʱ�ȭ 
        {
            cache1[i][j] = 0;
            cache2[i][j] = 0;
        }

    if (file == NULL) {// trace ������ �������� �ʴ´ٸ�. 
        perror("trace file State is NULL ! ");//perror �� ���� �Լ� 
        exit(0); // �����Լ�  
    }
    int bs = Bs; // Block size �񱳸� ���� ���� 
    while (bs != 0) { // Block Size�� 2�� n���϶� n�� ���ϱ� ������ �ݵ�� 2�� n�� �� �̾�� �� 
        if (bs & 1) cnt++; // ���� 2���� ���信�� �� ĭ�� 1�̸� cnt ���� 
        bs = bs >> 1; // ������ �������� ������ ��ĭ�� �̷�  
    }
    if (cnt != 1) { // Block Size�� 2�� n���϶� n�� ���ϱ� ������ �ݵ�� 2�� n�� �� �̾�� �� 
        perror("Block Size �� �ݵ�� 2�� ���� ��");
        exit(2);
    }
    if (!(Act & 1 || Act & 2 || Act & 4 || Act & 8)) {
        perror("������ ������ associativity�� 1,2,4,8 �� ���� ");
        exit(3);
    }

    if ((Cs % (Bs * Act)) != 0) { // �������� ���� �ִ� ��������.
         // ������ ������ cout�� ������� ��� �̴�. 
        perror("������ ������ cacheũ��� blockũ�� * associative�� ������� ��.");
        exit(4);
    }
    offsetCnt = BinaryCnt(Bs, 0);
    // Direct-Mapped �ΰ� N-way �ΰ��� �Ǻ�  
    if (Act != 1) indexCnt = BinaryCnt(Cs / Bs, 0) - BinaryCnt(Act, 0);
    else indexCnt = BinaryCnt(Cs / Bs, 0);

    tagCnt = 32 - indexCnt - offsetCnt;// tag = 32 - index -offset �̶�� ������ �̿� 
    //������ ǥ�� �ƿ� ǲ�� �̿�. 
    cout << "tag: " << tagCnt << " bits" << endl;
    cout << "index: " << indexCnt << " bits" << endl;
    cout << "offset: " << offsetCnt << " bits" << endl;

    pickIndex = pickIndex >> (tagCnt + offsetCnt); // pickIndex�� 32��Ʈ 1�ε�, �ε��� ����� �� �о����
    // �׷� index ��ŭ�� ũ���� ��Ʈ���� �� 1�� ���´�.  
    // �̰��� ���� �ؿ��� tagbit�� index�� �̾Ƴ��� ���� �� ������ ���� ���ȴ�. 


    while (!file.eof()) { // ������ ���� ��� ������. 
        bool  hitcheck = false; //hit ���� miss ������ �Ǵ��ϴ� üũ����. 
        char buffer[100]; // �ϴ� �� ���� �޾ƿ��� ���� ����. ���͸� �������� ���� �޾ƿ´�. 
        char temp[100] = { 0 }; // �ű⼭ ��� �ּҸ� �޾ƿ������� ����. �� ������ 
                             // ���� �ʿ��� ���� ��� �κ� ���̶� ��� �κ� ������ �� ��. 
        int bCnt = 0, comp = -1; // bCnt�� temp �迭�� ���� �� �ε����� ���� ����
                                // comp�� ���⸦ ���������� �����ϱ� �����̸�, ����� �����ϱ�����. 
        file.getline(buffer, 100); // �� �پ� �о���� �Լ�. buffer�� �� �� ���� �����Ѵ�. 
        for (int i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == ' ') comp *= -1; // ���⸦ �߰����� �� comp�� 1�� �ٲ�� ����κ� ����. 
            if (comp == 1) temp[bCnt++] = buffer[i]; //����κ� temp�� ����. 
        }
        tagbit = strtoul(temp, NULL, 16); // ������ ��� �ּҸ� 16������ ��ü( ���ڿ��̱� ���� ) 

        tagbit = tagbit >> offsetCnt; // offset bit �� �ϴ� ��� 
        index = tagbit & pickIndex;  // ������ �о���� pickIndex�� ������ index ���� 
        tagbit = tagbit >> indexCnt; // ���⼭ index bit �κ��� ���� ��μ� tag ��Ʈ �κи� ���´�. 

        total++; // ��Ż ���� 

        /* LRU �κ��Դϴ�. ������ ������ ĳ�ó� ����ִ� ĳ�ø� �����մϴ� */
        // hit �� ���  
        for (int i = 0; i < Act; i++) {
            if (cache1[index][i] == tagbit) {
                hit++;  // ������ ��Ʈ ! 
                hitcheck = true; // hit ���� miss ���� ���� Ȯ�� 
                cache2[index][i] = total; // cache2�� �� �ڸ��� ���� �ֽ����� ������Ʈ 
                break;
            }
        }
        // hit�� �߻����� �ʾ����� ( ��Ʈüũ�� false �϶� ) 
        if (hitcheck == false) {
            int minNum = 2100000000; //LRU �� ���� �� ĳ�ô� ������ ���ְ� �� á�� �� ���� ��  
            int flag = -1;             //ĳ�ø� ���� �ϱ� ���� �ּ� �� ã�� 
                                     //�� , ĳ��2�� 1 0 0 0�̸� 0�� �� ĳ���ε� flag�� 2���翡�� ���߰� �� ��
                               //�׷��� ����� �� �ű� ����, �ƴϸ� ���� ���� �� �༮ ���� 
            for (int i = 0; i < Act; i++) {
                if (cache2[index][i] < minNum) {
                    minNum = cache2[index][i]; // ����ų� ������ ĳ�� �˻� 
                    flag = i;  // �� ���� ������ ĳ�ð� �����. 
                }
            }
            cache2[index][flag] = total; // ��ų� ������ ĳ�ô� ������ ���� ���̱� ������ cache2�� total������ 
            cache1[index][flag] = tagbit; // �� �ڸ��� cache1 �� �ּҸ� ��� ĳ�ÿ� �ּ� �� ����. 
        }
    }
    hitrate = (double)hit / (double)total;
    cout.precision(2); // �Ҽ��� ��°�ڸ����� ���. 
    cout << "Result: total access :" << total << ", hit :" << hit << ", hit rate : " << hitrate << endl;//ǥ����� 
    return 0;
}