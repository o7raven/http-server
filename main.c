#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <Windows.h>

#define PORT "80"
#define DEFAULT_BUFLEN 1024

DWORD WINAPI handleConnection(LPVOID lpParameter);

int main(){
    WSADATA wsaData;
    int returnVal = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(returnVal!=0){
        printf("WSAStartup failed: %d\n", returnVal);
    }
    struct addrinfo *result = NULL, *ptr=NULL, hints; 
    ZeroMemory(&hints, sizeof(hints));
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
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    printf("Socket created successfully!\n");

    returnVal = bind(ListenSocket,result->ai_addr, (int)result->ai_addrlen);
    if(returnVal!=0){
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    printf("Socket binded successfully!\n");

    if(listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR){
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Listening on port %s...\n", PORT);
    for(;;){
        SOCKET* clientSocket = (SOCKET*)malloc(sizeof(SOCKET)); 
        *clientSocket = accept(ListenSocket, NULL, NULL);
        if(*clientSocket == INVALID_SOCKET){
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;
        }
        LPSECURITY_ATTRIBUTES threadId;
        HANDLE hThread = CreateThread(threadId, 0, handleConnection, (void*)clientSocket,0,NULL);
        if(hThread == NULL){
            printf("Thread creation has been unsuccessful");
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
    int bytesReceived = 0;
    if((recv(_clientSocket, buffer, DEFAULT_BUFLEN, 0)<=0)){
        closesocket(*_clientSocket);
        free(buffer);
        free(_clientSocket);
        return 1;
    }


    closesocket(*_clientSocket);
    free(buffer);
    free(_clientSocket);
    return 0;
}