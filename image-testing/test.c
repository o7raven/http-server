#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEFAULT_BUFLEN 1048576
static size_t getFileSize(FILE* file){
    int fileSize, currentPos;
    currentPos=ftell(file);
    fseek(file, 0,SEEK_END);
    fileSize = ftell(file);
    fseek(file,currentPos, SEEK_SET);
    return fileSize;
}
int main(){
    char* fileBuffer = malloc(DEFAULT_BUFLEN);
    FILE* fp = NULL;
    fp = fopen("borec.jpg", "rb");
    FILE* binary= fopen("image.bin", "wb");
    size_t fsize = getFileSize(fp);
    size_t bytesRead = fread(fileBuffer, sizeof(char),fsize , fp);
    printf("File size: %lu\n", fsize);
    fwrite(fileBuffer, sizeof(char), fsize,binary);
    fclose(binary);
    fclose(fp);
    return 0;
}
