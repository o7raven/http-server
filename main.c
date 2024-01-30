#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <sys/stat.h>
#include <time.h>
#include <stdio.h>

#define PORT "80"
#define DEFAULT_BUFLEN 1048576
#define MAX_URL_PATH_LENGTH 20

typedef enum{
    GET,
    POST,
    HEAD,
    PUT,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH
} REQUEST;
typedef struct{
    const char* extension;
    const char* mimeType;
} MIME; 
static const MIME mimeTypeList[] = {
    {"aac", "audio/aac"},
    {"abw", "application/x-abiword"},
    {"avi", "video/x-msvideo"},
    {"azw", "application/vnd.amazon.ebook"},
    {"bin", "application/octet-stream"},
    {"bmp", "image/bmp"},
    {"bz", "application/x-bzip"},
    {"bz2", "application/x-bzip2"},
    {"csh", "application/x-csh"},
    {"css", "text/css"},
    {"csv", "text/csv"},
    {"doc", "application/msword"},
    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"eot", "application/vnd.ms-fontobject"},
    {"epub", "application/epub+zip"},
    {"gif", "image/gif"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"ico", "image/vnd.microsoft.icon"},
    {"ics", "text/calendar"},
    {"jar", "application/java-archive"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"js", "text/javascript"},
    {"json", "application/json"},
    {"mid", "audio/midi"},
    {"midi", "audio/midi"},
    {"mpeg", "video/mpeg"},
    {"mpkg", "application/vnd.apple.installer+xml"},
    {"odp", "application/vnd.oasis.opendocument.presentation"},
    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {"odt", "application/vnd.oasis.opendocument.text"},
    {"oga", "audio/ogg"},
    {"ogv", "video/ogg"},
    {"ogx", "application/ogg"},
    {"otf", "font/otf"},
    {"png", "image/png"},
    {"pdf", "application/pdf"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {"rar", "application/x-rar-compressed"},
    {"rtf", "application/rtf"},
    {"sh", "application/x-sh"},
    {"svg", "image/svg+xml"},
    {"swf", "application/x-shockwave-flash"},
    {"tar", "application/x-tar"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},
    {"ttf", "font/ttf"},
    {"txt", "text"},
    {"vsd", "application/vnd.visio"},
    {"wav", "audio/wav"},
    {"weba", "audio/webm"},
    {"webm", "video/webm"},
    {"webp", "image/webp"},
    {"woff", "font/woff"},
    {"woff2", "font/woff2"},
    {"xhtml", "application/xhtml+xml"},
    {"xls", "application/vnd.ms-excel"},
    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"xml", "application/xml"},
    {"xul", "application/vnd.mozilla.xul+xml"},
    {"zip", "application/zip"},
    {"3gp", "video/3gpp"},
    {"3g2", "video/3gpp2"},
    {"7z", "application/x-7z-compressed"},
};

DWORD WINAPI handleConnection(LPVOID lpParameter);
static int getRequest(const char* buffer, const SSIZE_T bufferSize, REQUEST* request);
static int handleGETRequest(const char* buffer, const SOCKET* clientSocket);
static int createHTTPResponse(char* URI,char* responseBuffer, size_t* responseLen);
static const char* getMimeType(const char* extension);
static size_t getFileSize(FILE* file);

static const char* serverName = "CWebServer";
int main(){
    WSADATA wsaData;
    int returnVal = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(returnVal!=0){
        printf("WSAStartup failed: %d\n", returnVal);
        return 1;
    }
    struct addrinfo *result = NULL, hints={0}; 
    hints.ai_family=AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = INADDR_ANY;
    hints.ai_flags=AI_PASSIVE;
    returnVal = getaddrinfo(NULL, PORT, &hints, &result);

    if(returnVal!=0){
        printf("Getaddrinfo failed: %d\n", returnVal);
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(ListenSocket == INVALID_SOCKET){
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    puts("Socket created successfully!\n");

    returnVal = bind(ListenSocket,result->ai_addr, (int)result->ai_addrlen);
    if(returnVal!=0){
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    puts("Socket binded successfully!\n");

    if(listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR){
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("\nListening on port %s...\n\n", PORT);
    for(;;){
        SOCKET* clientSocket = malloc(sizeof(SOCKET)); 
        *clientSocket = accept(ListenSocket, NULL, NULL);
        if(*clientSocket == INVALID_SOCKET){
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;
        }
        HANDLE hThread = CreateThread(NULL, 0, handleConnection, (void*)clientSocket,0,NULL);
        if(hThread == NULL){
            printf("Thread creation has been unsuccessful\n");
            continue;
        }
        printf("Connection accepted\n");
        CloseHandle(hThread);
    }
    return 0;
}

DWORD WINAPI handleConnection(LPVOID lpParameter){
    SOCKET* _clientSocket = (SOCKET*)lpParameter;
    char* buffer = (char*)malloc(DEFAULT_BUFLEN);
    SSIZE_T bytesReceived = recv(*_clientSocket, buffer, DEFAULT_BUFLEN, 0);
    if(bytesReceived<0){
        closesocket(*_clientSocket);
        free(buffer);
        free(_clientSocket);

        return 1;
    }
    #ifdef DEBUG
        puts("Printing the request:");
        for(int i =0; i<bytesReceived; i++){
            printf("%c", buffer[i]);
        }
        puts("\n");
    #endif
    REQUEST request;
    if(getRequest(buffer, bytesReceived, &request) == 1){
        puts("Wrong request\n");
        closesocket(*_clientSocket);
        free(buffer);
        free(_clientSocket);
        return 1;
    }
    switch (request){
        case GET:
            if(handleGETRequest(buffer, (SOCKET*)lpParameter)==1){
                puts("Error handling the GET request\n");
                return 1;
            }
            break;
        
        default:
            puts("Request not supported\n");
            break;
    }
    closesocket(*_clientSocket);
    puts("Socket has been closed\n");
    free(buffer);
    free(_clientSocket);
    return 0;
}

static int getRequest(const char* buffer, const SSIZE_T bufferSize, REQUEST* request){
    //complicated solution due to security in buffer overflow
    size_t buffSize = 0;
    for(int i=0; i<bufferSize; i++){
        if(buffer[i] == ' ')
            break; 
        buffSize++;
    }
    char* requestPtr = malloc(buffSize);
    for(size_t i=0; i<buffSize; i++){
        requestPtr[i] = buffer[i];
    }
    if(strstr(requestPtr, "GET")){
        *request = GET;
    }else if(strstr(requestPtr, "POST")){
        *request = POST;
    }else{
        free(requestPtr);
        return 1;
    }
    free(requestPtr);
    return 0;
}

static int handleGETRequest(const char* buffer, const SOCKET* clientSocket){
    char* startPos = strchr(buffer, '/');
    char* URI = strtok(startPos, " ");
    #ifdef DEBUG
        puts("\nhandleGETRequest function start\n");
        printf("URI: %s\n", URI);
    #endif
    char* responseBuffer = malloc(DEFAULT_BUFLEN*2);
    size_t responseLen=0;
    if(createHTTPResponse(URI, responseBuffer, &responseLen)==1){
        puts("HTTP Response error\n");
    }
    #ifdef DEBUG
        printf("Response size: %d\n",responseLen);
        printf("Response buffer: \n%s\n", responseBuffer);
        puts("\nhandleGETRequest function end\n");
    #endif
    send(*clientSocket, responseBuffer, responseLen, 0);
    free(responseBuffer);
    return 0;
}

static int createHTTPResponse(char* URI,char* responseBuffer, size_t* responseLen){
    #ifdef DEBUG
        puts("createHTTPResponse start");
    #endif
    char* requestedFilePath = malloc(strlen(URI));
    strcpy(requestedFilePath, URI);
    requestedFilePath+=1;
    char* fileExt = strrchr(URI, '.');
    fileExt=fileExt+1;
    if(!fileExt){
        // Treat as a folder unless there's a custom URI
        #ifdef DEBUG
            puts("File extension not supported\n");
        #endif
    }
    const char* mime = getMimeType(fileExt);
    if(mime==NULL){
        puts("Unknown file extension\n");
    }
    #ifdef DEBUG
        printf("File extension: %s\n", fileExt);
        printf("MIME type: %s\n", mime);
    #endif

    FILE* requestedFile = fopen(requestedFilePath, "rb");
    if(requestedFile==NULL){
        snprintf(responseBuffer, DEFAULT_BUFLEN, 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: %s\r\n"
                "Server: %s\r\n"
                "\r\n"
                "404 Not Found", getMimeType("txt"), serverName);
        *responseLen = strlen(responseBuffer);
        free(requestedFilePath-1);
        fclose(requestedFile);
        return 1;
    }
    size_t requestedFileSize = getFileSize(requestedFile);
    #ifdef DEBUG
        printf("File size: %d\n", requestedFileSize);
    #endif
    char* fileBuffer = malloc(requestedFileSize);
    char* header  = malloc(DEFAULT_BUFLEN);
    
    struct stat fileStat;
    if(stat(requestedFilePath, &fileStat) == -1){
        printf("Error getting %s modification information", requestedFilePath);
        return 1;
    } 
    time_t lastModifiedTime = fileStat.st_mtime;
    struct tm *gmtTime=gmtime(&lastModifiedTime);
    size_t lastModifiedSize = 256;
    char* lastModified = malloc(lastModifiedSize);
    strftime(lastModified, lastModifiedSize, "%a, %d %b %Y %H:%M:%S GMT", gmtTime);

    snprintf(header, DEFAULT_BUFLEN, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Server: %s\r\n"
                "Last-Modified: %s\r\n"
                "Content-Length: %lu\r\n"
                "\r\n",mime,serverName,lastModified,(unsigned long)requestedFileSize);
    size_t bytesRead = fread(fileBuffer, sizeof(char), requestedFileSize, requestedFile);
    if(bytesRead != requestedFileSize){
        puts("Error reading the file\n");
        #ifdef DEBUG
            printf("Bytes read: %d\nFile size: %d\n", bytesRead, requestedFileSize);
        #endif
        free(fileBuffer);
        free(requestedFilePath-1);
        free(header);
        free(lastModified);
        fclose(requestedFile);
        return 1;
    }
    memcpy(responseBuffer, header, strlen(header));
    memcpy(responseBuffer+strlen(header), fileBuffer, requestedFileSize);
    *responseLen = strlen(header)+requestedFileSize;
    
    #ifdef DEBUG
        printf("\nHeader: \n%s\n", header);
        printf("Requested file path: %s\n", requestedFilePath);
        printf("Response buffer length: %d\n", *responseLen);
        puts("\ncreateHTTPResponse function end\n");
    #endif
    free(fileBuffer);
    free(requestedFilePath-1);
    free(header);
    free(lastModified);
    fclose(requestedFile);
    return 0; 
}
static const char* getMimeType(const char* extension){
    for(unsigned long long i = 0; i<sizeof(mimeTypeList)/sizeof(mimeTypeList[0]);i++){
        if(!strcmp(extension, mimeTypeList[i].extension)){
            return mimeTypeList[i].mimeType;
        }
    }
    return NULL;
}

static size_t getFileSize(FILE* file){
    int fileSize, currentPos;
    currentPos=ftell(file);
    fseek(file, 0,SEEK_END);
    fileSize = ftell(file);
    fseek(file,currentPos, SEEK_SET);
    return fileSize;
}