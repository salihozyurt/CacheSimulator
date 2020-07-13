#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define SIZE 255

typedef struct Cache{
    int id;
    char *cacheName;
    int setNum;
    char *tag;
    int time;
    int validBit;
    char *data;
    struct Cache *next;
}cache;

char *substring(char* resource,int start, int finish);
void LOneCache(char operator,char *hexa);
void Store(char *hexa, int byte, char *otherHexa);
void Modify (char *hexa, int byte, char *otherHexa);
int cacheControl(char *cacheName, int setNum);
void createCache(char *cacheName,int setNum);
void writeToLL();
void implementArguman(int length, char *arguman[]);
void readTraceFile();
char *hexaToBin (char *hexa);
int binToDec(char *bin);
void changeRamFile(int adressDec, char *otherHexa, int byteNum);
void loadRam();
char *readRamFile(int whereByte,int byteNum);
void writeRam();

cache *Header = NULL;
char *directory = NULL;
int id = 0,time = 0;
char ramFile[2100000] = {'\0'};
int lOneSize,lTwoSize,GlobalLOneSet,GlobalLTwoSet,GlobalLOneBlock,GlobalLTwoBlock;
int L1IHits = 0, L1IMisses = 0, L1IEvic = 0;
int L1DHits = 0, L1DMisses = 0, L1DEvic = 0;
int L2Hits = 0, L2Misses = 0, L2Evic = 0;

char *substring(char* resource,int start, int finish){
    int mines = finish - start;
    char *substring = (char *)calloc((size_t) (mines + 2), sizeof(char));
    strncpy(substring,resource+(start-1), (size_t) (mines + 1));
    return substring;
}

void LOneCache(char operator,char *hexa){
    char *hexaToB = hexaToBin(hexa);
    int setNum = binToDec(substring(hexaToB,(strlen(hexaToB))-(GlobalLOneSet+GlobalLOneBlock-1),(strlen(hexaToB))-GlobalLOneBlock));
    char *tag = substring(hexaToB,1,(strlen(hexaToB))-(GlobalLOneSet+GlobalLOneBlock));

    int setNumL2 = binToDec(substring(hexaToB,(strlen(hexaToB))-(GlobalLTwoSet+GlobalLTwoBlock-1),(strlen(hexaToB))-GlobalLTwoBlock));
    char *tagL2 = substring(hexaToB,1,(strlen(hexaToB))-(GlobalLTwoSet+GlobalLTwoBlock));

    char *cacheNameCont;
    if(operator == 'I'){
        cacheNameCont = "L1I";
    } else {
        cacheNameCont = "L1D";
    }

    cache *p,*q,*r;
    int control = 0;
    int controlL2 = 0;

    p=Header;
    while (p != NULL) {
        if(strcmp(p->cacheName,cacheNameCont) == 0 && p->validBit == 1 && p->setNum == setNum && strcmp(p->tag,tag) == 0){
            control = 1;
            if(strcmp(cacheNameCont,"L1I") == 0){
                printf("\tL1I hit in set %d\n",p->setNum);
                L1IHits++;
            } else if(strcmp(cacheNameCont,"L1D") == 0){
                printf("\tL1D hit in set %d\n",p->setNum);
                L1DHits++;
            }
        }
        p=p->next;
    }
    if(control == 0){
        if(strcmp(cacheNameCont,"L1I") == 0){
            printf("\tL1I miss, ");
            L1IMisses++;
        } else if(strcmp(cacheNameCont,"L1D") == 0){
            printf("\tL1D miss, ");
            L1DMisses++;
        }
        q=Header;
        while (q != NULL) {
            if(strcmp(q->cacheName,"L2") == 0 && q->validBit == 1 && q->setNum == setNumL2 && strcmp(q->tag,tagL2) == 0){
                controlL2 = 1;
                printf("L2 hit. \n");
                L2Hits++;
                int dataL1Id = cacheControl(cacheNameCont,setNum);
                r=Header;
                while (r != NULL) {
                    if(r->id == dataL1Id){
                        time++;
                        printf("\tLoad in %s set %d from L2 set %d\n",r->cacheName,r->setNum,q->setNum);
                        r->tag = tag; r->time = time; r->validBit = 1; r->data = q->data;
                    }
                    r=r->next;
                }
            }
            q=q->next;
        }
        if(controlL2 == 0){
            printf("L2 miss. \n");
            L2Misses++;
            char *data = readRamFile(binToDec(hexaToB),lTwoSize);
            int dataL2Id = cacheControl("L2",setNumL2);
            r=Header;
            while (r != NULL) {
                if(r->id == dataL2Id){
                    time++;
                    printf("\tLoad in %s set %d from RAM\n",r->cacheName,r->setNum);
                    r->tag = tagL2; r->time = time; r->validBit = 1; r->data = data;
                }
                r=r->next;
            }
            char *dataL1 = readRamFile(binToDec(hexaToB),lOneSize);
            int dataL1Id = cacheControl(cacheNameCont,setNum);
            r=Header;
            while (r != NULL) {
                if(r->id == dataL1Id){
                    time++;
                    printf("\tLoad in %s set %d from RAM\n",r->cacheName,r->setNum);
                    r->tag = tag; r->time = time; r->validBit = 1; r->data = dataL1;
                }
                r=r->next;
            }
        }
    }
}

void Store(char *hexa, int byte, char *otherHexa){
    char *hexaToB = hexaToBin(hexa);
    int setNum = binToDec(substring(hexaToB,(strlen(hexaToB))-(GlobalLOneSet+GlobalLOneBlock-1),(strlen(hexaToB))-GlobalLOneBlock));
    char *tag = substring(hexaToB,1,(strlen(hexaToB))-(GlobalLOneSet+GlobalLOneBlock));

    int setNumL2 = binToDec(substring(hexaToB,(strlen(hexaToB))-(GlobalLTwoSet+GlobalLTwoBlock-1),(strlen(hexaToB))-GlobalLTwoBlock));
    char *tagL2 = substring(hexaToB,1,(strlen(hexaToB))-(GlobalLTwoSet+GlobalLTwoBlock));

    cache *p;
    int control = 0;
    int controlL2 = 0;

    p=Header;
    while (p != NULL) {
        if(strcmp(p->cacheName,"L1D") == 0 && p->validBit == 1 && p->setNum == setNum && strcmp(p->tag,tag) == 0){
            control = 1;
            printf("\tL1D hit, ");
            L1DHits++;
            char *pData = p->data;
            char *subPData = substring(pData,(byte*2)+1,strlen(pData));
            char continueData[SIZE];
            strcpy(continueData,otherHexa);
            strcat(continueData,subPData);
            strcpy(p->data,continueData);
            free(pData);free(subPData);
        }
        p=p->next;
    }
    if(control == 0){printf("\tL1D miss, "); L1DMisses++;}
    p=Header;
    while (p != NULL) {
        if(strcmp(p->cacheName,"L2") == 0 && p->validBit == 1 && p->setNum == setNumL2 && strcmp(p->tag,tagL2) == 0){
            controlL2 = 1;
            printf("L2 hit. \n");
            L2Hits++;
            char *pData = p->data;
            char *subPData = substring(pData,(byte*2)+1,strlen(pData));
            char continueData[SIZE];
            strcpy(continueData,otherHexa);
            strcat(continueData,subPData);
            strcpy(p->data,continueData);
            free(pData);free(subPData);
        }
        p=p->next;
    }
    if(controlL2 == 0){printf("L2 miss. \n"); L2Misses++;}

    if(control == 1 && controlL2 == 1){
        printf("\tStore in L1D, L2 , RAM\n");
    } else if(control == 0 && controlL2 == 1){
        printf("\tStore in L2 , RAM\n");
    } else if(control == 1 && controlL2 == 0){
        printf("\tStore in L1D, RAM\n");
    } else if(control == 0 && controlL2 == 0) {
        printf("\tStore in RAM\n");
    }

    changeRamFile(binToDec(hexaToB),otherHexa,byte);

}

void Modify (char *hexa, int byte, char *otherHexa){
    LOneCache('L',hexa);
    Store(hexa,byte,otherHexa);
}

int cacheControl(char *cacheName, int setNum){
    cache *p,*q;
    int control = 0;
    int returnedId = 0;

    int newtime, oldtime = 0;
    int newId,oldId;
    int controlTime;

    p=Header;
    while (p != NULL) {
        if(strcmp(p->cacheName,cacheName)== 0 && p->validBit == 0 && p->setNum == setNum){
            control = 1;
            returnedId = p->id;
            break;
        }
        p=p->next;
    }
    if(control == 0){
        if(strcmp(cacheName,"L1I") == 0){
            L1IEvic++;
        } else if(strcmp(cacheName,"L1D") == 0){
            L1DEvic++;
        } else {
            L2Evic++;
        }
        q=Header;
        while (q != NULL) {
            if(strcmp(q->cacheName,cacheName)== 0 && q->setNum == setNum){
                newtime = q->time;
                newId = q->id;
                if(controlTime == 1){
                    if (newtime > oldtime){
                        returnedId = oldId;
                    } else {
                        returnedId = newId;
                    }
                }
                oldtime = newtime;
                oldId = newId;
                controlTime = 1;
            }
            q=q->next;
        }
    }
    return returnedId;
}

void createCache(char *cacheName,int setNum){
    id++;
    cache *p,*q;

    p = (cache*)malloc(sizeof(cache));
    p->id = id;
    p->cacheName = cacheName;
    p->setNum = setNum;
    p->tag = "";
    p->time = 0;
    p->validBit = 0;
    p->data = "";
    p->next = NULL;

    if(Header==NULL){
        Header=p;
    }else {
        q=Header;
        while(q->next!=NULL){
            q=q->next;
        }
        q->next=p;
    }

}

void writeToLL(){
    cache *p;

    p=Header;
    while (p != NULL) {
        printf("%s\t\t%d\t\t%s\t%d\t%d\t\t%s\n",p->cacheName,p->setNum,p->tag,p->time,p->validBit,p->data);
        p=p->next;
    }
}

void implementArguman(int length, char *arguman[]){

    int lOneSet,lOneLine,lTwoSet,lTwoLine;

    for (int i = 0; i < length; ++i) {
        if(strcmp(arguman[i],"-L1s") == 0){
            GlobalLOneSet = atoi(arguman[i + 1]);
            lOneSet = (int) pow(2, atoi(arguman[i + 1]));
        }else if (strcmp(arguman[i],"-L1E")== 0){
            lOneLine = atoi(arguman[i+1]);
        }else if (strcmp(arguman[i],"-L1b")== 0){
            GlobalLOneBlock = atoi(arguman[i + 1]);
            lOneSize = (int) pow(2, atoi(arguman[i + 1]));
        }else if (strcmp(arguman[i],"-L2s")== 0){
            GlobalLTwoSet = atoi(arguman[i + 1]);
            lTwoSet = (int) pow(2, atoi(arguman[i + 1]));
        }else if (strcmp(arguman[i],"-L2E")== 0){
            lTwoLine = atoi(arguman[i+1]);
        }else if (strcmp(arguman[i],"-L2b")== 0){
            GlobalLTwoBlock = atoi(arguman[i + 1]);
            lTwoSize = (int) pow(2, atoi(arguman[i + 1]));
        }else if (strcmp(arguman[i],"-t")== 0){
            directory = arguman[i+1];
        }
    }

    for(int i = 0; i < lOneSet; i++){
        for(int j = 0; j < lOneLine; j++){
            createCache("L1I",i);
        }
    }

    for(int i = 0; i < lOneSet; i++){
        for(int j = 0; j < lOneLine; j++){
            createCache("L1D",i);
        }
    }

    for(int i = 0; i < lTwoSet; i++){
        for(int j = 0; j < lTwoLine; j++){
            createCache("L2",i);
        }
    }
}

void readTraceFile(){
    char trace[SIZE];
    char instractionChar;
    char *hexaAdress;
    int byteSize;
    char *otherHexaAdress;

    printf("----------------------------------------------------------------------------------------------------\n");

    FILE *inputFile;
    inputFile=fopen(directory,"r");
    while (!feof(inputFile)) {
        fgets(trace,SIZE,inputFile);
        printf("%s",trace);
        instractionChar = trace[0];
        hexaAdress = substring(trace,3,10);
        byteSize = atoi(substring(trace,13,13));

        switch (instractionChar){
            case 'I':
            case 'L':
                LOneCache(instractionChar,hexaAdress);
                break;
            case 'S':
                otherHexaAdress = substring(trace,16,(strlen(trace)-1));
                Store(hexaAdress,byteSize,otherHexaAdress);
                break;
            case 'M':
                otherHexaAdress = substring(trace,16,(strlen(trace)-1));
                Modify(hexaAdress,byteSize,otherHexaAdress);
                break;
            default:
                break;
        }
        strcpy(trace,"");
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    fclose(inputFile);
}

char *hexaToBin (char *hexa){
    int leng = strlen(hexa);
    char bin[SIZE] = "";
    for (int i = 0; i < leng; ++i) {
        switch(hexa[i]){
            case '0':
                strcat(bin, "0000");
                break;
            case '1':
                strcat(bin, "0001");
                break;
            case '2':
                strcat(bin, "0010");
                break;
            case '3':
                strcat(bin, "0011");
                break;
            case '4':
                strcat(bin, "0100");
                break;
            case '5':
                strcat(bin, "0101");
                break;
            case '6':
                strcat(bin, "0110");
                break;
            case '7':
                strcat(bin, "0111");
                break;
            case '8':
                strcat(bin, "1000");
                break;
            case '9':
                strcat(bin, "1001");
                break;
            case 'a':
            case 'A':
                strcat(bin, "1010");
                break;
            case 'b':
            case 'B':
                strcat(bin, "1011");
                break;
            case 'c':
            case 'C':
                strcat(bin, "1100");
                break;
            case 'd':
            case 'D':
                strcat(bin, "1101");
                break;
            case 'e':
            case 'E':
                strcat(bin, "1110");
                break;
            case 'f':
            case 'F':
                strcat(bin, "1111");
                break;
            default:
                break;
        }
    }
    char *binP = strdup(bin);
    return binP;
}

int binToDec(char *bin){
    int leng = strlen(bin);
    int dec = 0;
    for (int i = leng - 1; i >= 0; i--) {
        if(bin[i] == '1'){
            dec = (int) (dec + pow(2, leng - i - 1));
        }
    }
    return dec;
}

void changeRamFile(int adressDec, char *otherHexa, int byteNum){

    int counter = 0;
    for(int i = (adressDec-1)*2; i < ((adressDec-1)*2+(byteNum*2));i++){
        ramFile[i] = otherHexa[counter];
        counter++;
    }

}

void loadRam(){
    char ram;
    int counter = 0;
    FILE *inputFile;
    inputFile = fopen("ram.txt","r");
    while (!feof(inputFile)) {
        fscanf(inputFile,"%c",&ram);
        if (ram != '\n' && ram!= ' '){
            ramFile[counter] = ram;
            counter++;
        }
    }
    fclose(inputFile);
}

char *readRamFile(int whereByte,int byteNum){
    char returnRam[SIZE] = "";
    int counter = 0;
    for(int i = (whereByte-1)*2; i < ((whereByte-1)*2+(byteNum*2));i++){
        returnRam[counter] = ramFile[i];
        counter++;
    }
    char *ramP = strdup(returnRam);
    return ramP;
}

void writeRam(){
    FILE *outputFile;
    outputFile = fopen("ram.txt","w");
    for(int i = 0; i < 2100000; i++){
        if (ramFile[i] != '\0'){
            fprintf(outputFile,"%c",ramFile[i]);
            if((i+1) % 2 == 0){
                fprintf(outputFile,"%c",' ');
            }
        }
    }
    fclose(outputFile);
}

void main(int argc, char *argv[]) {

    loadRam();
    implementArguman(argc, argv);
    readTraceFile();
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("THE LAST VIEW OF CACHES\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("Cache Name\tSet Index \tTag\t\t\t\tTime\tValid Bit\tData\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    writeToLL();
    printf("----------------------------------------------------------------------------------------------------\n");
    writeRam();

    printf("----------------------------------------------------------------------------------------------------\n");
    printf("TOTAL HITS, MISSES AND EVICTIONS NUMBER OF CACHES\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("L1I Total Hits : \t%d\t L1I Total Misses : \t%d\t L1I Total Evictions : \t%d\t \n",L1IHits,L1IMisses,L1IEvic);
    printf("L1D Total Hits : \t%d\t L1D Total Misses : \t%d\t L1D Total Evictions : \t%d\t \n",L1DHits,L1DMisses,L1DEvic);
    printf("L2 Total Hits : \t%d\t L2 Total Misses : \t%d\t L2 Total Evictions : \t%d\t \n",L2Hits,L2Misses,L2Evic);
    printf("----------------------------------------------------------------------------------------------------\n");

}