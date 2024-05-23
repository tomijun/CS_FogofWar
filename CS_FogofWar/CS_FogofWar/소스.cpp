#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"

#define Black(X,Y) ((X)|=(1<<(Y)))
#define White(X,Y) ((X)&=(0xFF - (1<<(Y))))

typedef unsigned long long QWORD;
MPQHANDLE* Save1[1024];
FILE* Save2[1024];
int Save1ptr = 0;
int Save2ptr = 0;
#define PI 3.14159265

void f_ScloseAll(MPQHANDLE hMPQ)
{
    for (int i = 0; i < Save2ptr; i++)
        fclose(Save2[i]);
    for (int i = 0; i < Save1ptr; i++)
        SFileCloseFile(*Save1[i]);
    SFileCloseArchive(hMPQ); // MPQ 닫기
    Save1ptr = 0;
    Save2ptr = 0;
    return;
}

int f_Sopen(MPQHANDLE hMPQ, LPCSTR lpFileName, MPQHANDLE* hFile)
{
    // SFileOpenFileEx([MPQ핸들], [파일명], 0, &[리턴받을 파일핸들]);
    if (!SFileOpenFileEx(hMPQ, lpFileName, 0, hFile)) {
        printf("%s이(가) 존재하지 않습니다. [%d]\n", lpFileName, GetLastError());
        return -1;
    }
    Save1[Save1ptr++] = hFile;
}

int f_Scopy(MPQHANDLE hMPQ, MPQHANDLE* hFile, LPCSTR foutName, FILE** fout)
{
    fopen_s(fout, foutName, "w+b");
    if (*fout == NULL) {
        f_ScloseAll(hMPQ);
        printf("%s을 열 수 없습니다.\n", foutName);
        return -1;
    }
    Save2[Save2ptr++] = *fout;
    DWORD fsize = SFileGetFileSize(*hFile, NULL);
    char buffer[4096] = { 0 };
    while (fsize > 0) {
        DWORD transfersize = min(4096, fsize);
        DWORD readbyte;
        SFileReadFile(*hFile, buffer, transfersize, &readbyte, NULL); //fread에 해당
        if (readbyte != transfersize) {
            printf("SFileReadFile read %d bytes / %d bytes expected.\n", readbyte, transfersize);
            f_ScloseAll(hMPQ);
            return -1;
        }
        fwrite(buffer, 1, readbyte, *fout);
        fsize -= transfersize;
    }
}

int f_Fcopy(FILE** fin, FILE** fout, DWORD fsize)
{
    FILE* Fin = *fin;
    FILE* Fout = *fout;

    char buffer[4096] = { 0 };

    while (fsize > 0) {
        DWORD transfersize = min(4096, fsize);
        DWORD readbyte;
        readbyte = fread(buffer, transfersize, 1, Fin);
        if (readbyte != 1) {
            printf("ReadFile Failed.\n");
            return -1;
        }
        fwrite(buffer, transfersize, 1, Fout);
        fsize -= transfersize;
    }
}

void f_Swrite(MPQHANDLE hMPQ, LPCSTR finName, LPCSTR MPQName)
{
    MpqAddFileToArchive(hMPQ, finName, MPQName, MAFA_COMPRESS);
}

void f_SwriteWav(MPQHANDLE hMPQ, LPCSTR finName, LPCSTR MPQName)
{
    MpqAddWaveToArchive(hMPQ, finName, MPQName, MAFA_COMPRESS, MAWA_QUALITY_LOW);
}

int getTotalLine(FILE* fp) {
    int line = 0;
    char c;
    while ((c = fgetc(fp)) != EOF)
        if (c == '\n') line++;
    return(line);
}

int GetChkSection(FILE* fp, const char* Name, DWORD* Ret1, DWORD* Ret2)
{
    DWORD Key = Name[0] + Name[1] * 256 + Name[2] * 65536 + Name[3] * 16777216;
    int ret = 0, size;
    fseek(fp, 0, 2);
    size = ftell(fp);
    fseek(fp, 0, 0);

    DWORD Section[2];
    DWORD Check = 0;
    for (int i = 0; i < size;)
    {
        fseek(fp, i, 0);
        fread(Section, 4, 2, fp);
        if (Section[0] == Key)
        {
            *Ret1 = i;
            *Ret2 = i + (Section[1] + 8);
            Check = 1;
            break;
        }
        else
            i += (Section[1] + 8);
    }
    if (Check == 0)
        printf("맵의 %s단락을 찾을 수 없습니다.", Name);
    return(*Ret2) - (*Ret1);
}


char* Find_Next(char* ptr)
{
    char* temp = ptr;
    while (1)
    {
        if (*temp == 0)
            return NULL;
        else if (*temp == ' ')
            break;
        temp++;
    }

    while (1)
    {
        if (*temp == 0)
            return NULL;
        else if ((*temp >= '0' && *temp <= '9') || (*temp >= 'a' && *temp <= 'z') || (*temp >= 'A' && *temp <= 'Z'))
        {
            ptr = temp;
            break;
        }
        temp++;
    }
    return ptr;
}


void compare(char* Input, int* op)
{
    char tempstr[16] = { 0 };
    strncpy_s(tempstr, 16, Input, 3);
    if (!strncmp(Input, "P1", 2) || !strncmp(Input, "p1", 2) || !strncmp(Input, "0", 1))
        op[0] = 1;
    else if (!strncmp(Input, "P2", 2) || !strncmp(Input, "p2", 2) || !strncmp(Input, "1", 1))
        op[1] = 1;
    else if (!strncmp(Input, "P3", 2) || !strncmp(Input, "p3", 2) || !strncmp(Input, "2", 1))
        op[2] = 1;
    else if (!strncmp(Input, "P4", 2) || !strncmp(Input, "p4", 2) || !strncmp(Input, "3", 1))
        op[3] = 1;
    else if (!strncmp(Input, "P5", 2) || !strncmp(Input, "p5", 2) || !strncmp(Input, "4", 1))
        op[4] = 1;
    else if (!strncmp(Input, "P6", 2) || !strncmp(Input, "p6", 2) || !strncmp(Input, "5", 1))
        op[5] = 1;
    else if (!strncmp(Input, "P7", 2) || !strncmp(Input, "p7", 2) || !strncmp(Input, "6", 1))
        op[6] = 1;
    else if (!strncmp(Input, "P8", 2) || !strncmp(Input, "p8", 2) || !strncmp(Input, "7", 1))
        op[7] = 1;
    else if (!_stricmp(tempstr, "all"))
        for (int i = 0; i < 8; i++) op[i] = 1;
}

void ParseMsg(char* Input, int* op)
{
    char* front = Input, * rear; // front ~ rear : 
    for (int i = 0; i < 8; i++)
        op[i] = 0;

    int check = 0;
    for (int i = 0; i < 9; i++)
    {
        rear = Find_Next(front);
        if (rear == NULL)
        {
            check = 1;
            if (front == NULL) break;
        }

        compare(front,op);      
        front = rear;

        if (check == 1) break;
    }
    return;
}


int main(int argc, char* argv[])
{
    FILE* fout, * lout;
    FILE* wout[1024];
    MPQHANDLE hMPQ;
    MPQHANDLE hFile;
    MPQHANDLE hList;
    MPQHANDLE hWav[1024];
    char* WavName[1024];
    int Wavptr = 0;

    // Open MPQ

    printf("--------------------------------------\n     。`+˚CS_FogofWar v1.0 。+.˚\n--------------------------------------\n\t\t\tMade By Ninfia\n");

    char* input = argv[1];
    //Test
    //char input2[] = "1.scx";
    //if (argc == 1)
    //    input = input2;
    //Test

    
    if (argc == 1) // Selected No file
    {
        printf("선택된 파일이 없습니다.\n");
        system("pause");
        return 0;
    }
    

    char iname[512];
    strcpy_s(iname, 512, input);
    int ilength = strlen(iname) - 4;
    if (ilength < 0) return 0;
    strcpy_s(iname + ilength, 512 - ilength, "_out.scx");


    if (!SFileOpenArchive(input, 0, 0, &hMPQ)) {
        // SFileOpenArchive([파일명], 0, 0, &[리턴받을 MPQ핸들]);
        printf("SFileOpenArchive failed. [%d]\n", GetLastError());
        return -1;
    }
    printf("%s 의 MPQ 로드 완료\n", input);
    // Open Files
    f_Sopen(hMPQ, "(listfile)", &hList);
    f_Scopy(hMPQ, &hList, "(listfile).txt", &lout);
    printf("(listfile)을 불러와 맵 내부의 파일 목록을 읽는중\n");
    fseek(lout, 0, 0);
    char strTemp[512] = { 0 };
    int strLength, listline, line;
    listline = getTotalLine(lout);
    line = 0;
    fseek(lout, 0, 0);
    int chksize = 0;
    while (line < listline)
    {
        line++;
        fgets(strTemp, 512, lout);
        strLength = strlen(strTemp);
        strTemp[strLength - 2] = 0;
        if (!strcmp(strTemp, "staredit\\scenario.chk"))
        {
            f_Sopen(hMPQ, "staredit\\scenario.chk", &hFile);
            f_Scopy(hMPQ, &hFile, "scenario.chk", &fout);
            fseek(fout, 0, 2);
            chksize = ftell(fout);
            fseek(fout, 0, 0);
            fclose(fout);
            SFileCloseFile(hFile);
        }
        else
        {
            char* tmpBuffer = (char*)malloc(512);
            tmpnam_s(tmpBuffer, 512);
            WavName[Wavptr] = tmpBuffer;
            f_Sopen(hMPQ, strTemp, &hWav[Wavptr]);
            f_Scopy(hMPQ, &hWav[Wavptr], WavName[Wavptr], &wout[Wavptr]);
            fclose(wout[Wavptr]);
            SFileCloseFile(hWav[Wavptr]);
            Wavptr++;
        }

    }
    fclose(lout);
    SFileCloseFile(hList);
    SFileCloseArchive(hMPQ); // MPQ 닫기

    printf("scenario.chk %d bytes, 사운드 %d개 추출됨\n\n", chksize, Wavptr);


    DWORD JumpSize[10] = { 0 };
    fopen_s(&fout, "scenario.chk", "rb");
    FILE* fnew, * fMASK, * fDIM;
    fopen_s(&fnew, "scenario_new.chk", "wb");

    DWORD DIMoff1, DIMoff2, DIMsize = GetChkSection(fout, "DIM ", &DIMoff1, &DIMoff2);

    fopen_s(&fDIM, "DIM.chk", "w+b");
    fseek(fout, DIMoff1, 0);
    f_Fcopy(&fout, &fDIM, DIMsize);
    DWORD MapX = 0, MapY = 0, temp;
    fseek(fDIM, 8, 0);
    fread(&MapX, 2, 1, fDIM);
    fread(&MapY, 2, 1, fDIM);

    printf("%d x %d 크기의 지형 로드됨\n", MapX, MapY);
    char Input[512] = { 0 }, image[512] = {0};
    int op[8];

    BYTE** CurMask = (BYTE**)malloc(MapY * sizeof(BYTE*));
    for (int i = 0; i < MapY; i++)
        CurMask[i] = (BYTE*)malloc(MapX * sizeof(BYTE));
    
    for (int i = 0; i < MapY; i++)
        for (int j = 0; j < MapX; j++)
            CurMask[i][j] = 0xFF;
    do
    {
        printf("전장의 안개에 삽입할 이미지 파일을 입력 (0:종료)\n");
        scanf_s("%s", image,512); getchar();

        if (!strcmp(image, "0"))
            break;
        else
        {
            // BMP Size : XxY 단색 팔레트 
            BITMAP BMP;
            HBITMAP HBMP = (HBITMAP)LoadImageA(0, image, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            GetObject(HBMP, sizeof(BMP), (LPSTR)&BMP);
            BYTE* IPtr = (BYTE*)BMP.bmBits;
            DWORD X = BMP.bmWidth, Y = BMP.bmHeight;

            if (BMP.bmBitsPixel != 8)
            {
                printf("8비트 BMP가 아닙니다. 다른 이미지를 선택해주세요\n\n");
            }
            else if (X != MapX || Y != MapY)
            {
                printf("이미지의 크기가 %dx%d가 아닙니다. 다른 이미지를 선택해주세요\n\n", MapX, MapY);
            }
            else
            {
                printf("%s 로드완료 (%dx%d), 8Bit Color BMP)\n\n", image, X, Y);

                DWORD NX, X3 = (X) % 4;
                if (X3 == 0)
                    NX = X;
                else if (X3 == 1)
                    NX = X + 3;
                else if (X3 == 2)
                    NX = X + 2;
                else
                    NX = X + 1;

                BYTE* BPtr = (BYTE*)BMP.bmBits;

                printf("삽입할 플레이어(P1~P8, ALL)를 입력\n");
                scanf_s("%[^\n]", Input, 512);  getchar();
                ParseMsg(Input, op);

                for (int j = 0; j < Y; j++)
                {
                    for (int i = 0; i < X; i++)
                    {
                        int temp = j * NX + i;
                        if (BPtr[temp] == 1) // White
                        {
                            for (int k = 0; k < 8; k++)
                                if (op[k] == 1)
                                    Black(CurMask[j][i], k);
                        }
                        else // Black
                        {
                            for (int k = 0; k < 8; k++)
                                if(op[k] == 1)
                                    White(CurMask[j][i], k);
                        }
                    }
                }

                int check = 0, init = 0;
                printf("→%s 를 ", image);
                for (int i = 0; i < 8; i++)
                    if (op[i] == 1)
                    {
                        if (init == 1 && check == 1)
                        {   
                            check = 0;
                            printf(", ");
                        }
                        printf("P%d", i + 1);
                        check = 1;
                        init = 1;
                    }
                printf("의 전장의 안개에 삽입했습니다.\n\n");
            }
            DeleteObject(HBMP);
        }
    } while (1);

    fseek(fout, 0, 2);
    int fsize = ftell(fout);
    fseek(fout, 0, 0);
    const char* Pass[] = { "MASK" };
    DWORD Section[2];
    for (int i = 0; i < fsize;)
    {
        fseek(fout, i, 0);
        fread(Section, 4, 2, fout);

        DWORD Offset, check = 0;
        for (int j = 0; j < 1; j++)
        {
            DWORD Key = Pass[j][0] + (Pass[j][1] << 8) + (Pass[j][2] << 16) + (Pass[j][3] << 24);
            if (Section[0] == Key) check = 1;
        }

        if (check == 0) // copy section
        {
            Offset = i;
            fseek(fout, Offset, 0);
            f_Fcopy(&fout, &fnew, Section[1] + 8); // Copy UNIT
        }
        i += (Section[1] + 8);
    }

    // make MASK
    fopen_s(&fMASK, "MASK.chk", "w+b");
    DWORD MASKsize = MapX * MapY + 8;
    temp = (int)'M' + (int)'A' * 256 + (int)'S' * 65536 + (int)'K' * 16777216;
    fwrite(&temp, 4, 1, fMASK);
    temp = MASKsize - 8;
    fwrite(&temp, 4, 1, fMASK);
    for (int i = MapY-1; i >= 0; i--)
        for (int j = 0; j < MapX; j++)
            fwrite(&CurMask[i][j], 1, 1, fMASK);

    fseek(fMASK, 0, 0);
    f_Fcopy(&fMASK, &fnew, MASKsize); // Copy MASK

    printf("출력할 전장의 안개파일 데이터의 이름을 입력하세요 (0 입력시 출력안함) : ");
    char Export[512] = { 0 };
    scanf_s("%s", Export, 512);
    if (strcmp(Export, "0"))
    {
        strcat_s(Export, 512, "_MASK.chk");
        FILE* EXChk;
        fopen_s(&EXChk, Export, "w+b");
        fseek(fMASK, 8, 0);
        f_Fcopy(&fMASK, &EXChk, MASKsize - 8);
        fclose(EXChk);
        printf("%s로 유닛파일 데이터가 출력됨 (0x%X bytes)\n\n", Export, MASKsize-8);
    }

    chksize = ftell(fnew);
    fclose(fout);
    fclose(fnew);
    fclose(fMASK);
    fclose(fDIM);

    DeleteFileA("MASK.chk");
    DeleteFileA("DIM.chk");

    // Write MPQ
    char* out = iname;
    hMPQ = MpqOpenArchiveForUpdate(out, MOAU_CREATE_ALWAYS, 1024);
    if (hMPQ == INVALID_HANDLE_VALUE) { DeleteFileA(out);  return false; }

    // Write Files & Delete Temp
    f_Swrite(hMPQ, "(listfile).txt", "(listfile)");
    fopen_s(&lout, "(listfile).txt", "rb");

    Wavptr = 0;
    line = 0;
    while (line < listline)
    {
        line++;
        fgets(strTemp, 512, lout);
        strLength = strlen(strTemp);
        strTemp[strLength - 2] = 0;
        if (!strcmp(strTemp, "staredit\\scenario.chk"))
        {
            f_Swrite(hMPQ, "scenario_new.chk", "staredit\\scenario.chk");
            DeleteFileA("scenario.chk");
            DeleteFileA("scenario_new.chk");
        }
        else
        {
            f_SwriteWav(hMPQ, WavName[Wavptr], strTemp);
            DeleteFileA(WavName[Wavptr]);
            free(WavName[Wavptr]);
            Wavptr++;
        }
    }
    fclose(lout);
    DeleteFileA("(listfile).txt");
    MpqCloseUpdatedArchive(hMPQ, 0);

    printf("적용후 scenario.chk 의 크기 : %dbytes\n%s 로 저장됨\a\n", chksize, iname);

    system("pause");
    return 0;
}
