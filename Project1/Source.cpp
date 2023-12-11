#include <iostream>
#include <thread>
#include <fstream>
#include <string>


#include <winsock2.h>
#include <Ws2tcpip.h>
#include <tchar.h>
#pragma comment(lib, "ws2_32.lib")

const int DEFAULT_PORT = 12345;

using namespace std;
const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\ChatPipe";
const wchar_t* mailslotName = L"\\\\.\\mailslot\\ChatMailSlot";

std::string types[3] = { "Pipes", "Sockets", "Mailbox" };



#include "Pipes.h"
#include "Sockets.h"
#include "MailSlots.h"




void Server() {

   // std::ofstream outputFile("../x64/Debug/number.txt");

    std::ofstream outputFile("C:/Users/voinv/Desktop/Polytechnik/OS/Lab 9/Project1/number.txt");

    int choice;
    if (outputFile.is_open()) {
        std::cout << "Choose method (1 - Pipes, 2 - Sockets, 3 - MailSlots): ";
    }
    else {
        std::cout << "Unable to open file for writing." << std::endl;
        return; 
    }
    std::cin >> choice;
    outputFile << choice << std::endl;
    outputFile.close();
    if (choice >= 1 && choice <= 3) {
        std::cout << "You method is: " << types[choice - 1] << endl;
    }
    else {
        std::cout << "Incorrect value";
    }

    if (choice == 1) {
        HANDLE pipe = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            PIPE_UNLIMITED_INSTANCES,
            0, 0, 0, NULL
        );

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipe failed, GLE=" << GetLastError() << std::endl;
            return;
        }

        std::cout << "Server waiting for client..." << std::endl;

        HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!event) {
            std::cerr << "CreateEvent failed, GLE=" << GetLastError() << std::endl;
            CloseHandle(pipe);
            return;
        }

        OVERLAPPED overlapped = { 0 };
        overlapped.hEvent = event;

        if (!ConnectNamedPipe(pipe, &overlapped)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                std::cerr << "ConnectNamedPipe failed, GLE=" << GetLastError() << std::endl;
                CloseHandle(pipe);
                CloseHandle(event);
                return;
            }
        }

        DWORD bytesTransferred;
        if (!GetOverlappedResult(pipe, &overlapped, &bytesTransferred, TRUE)) {
            std::cerr << "GetOverlappedResult failed, GLE=" << GetLastError() << std::endl;
            CloseHandle(pipe);
            CloseHandle(event);
            return;
        }

        std::cout << "Client connected. Start chatting!" << std::endl;

        
        std::thread serverThread(ServerThreadOnPipes, pipe);

        while (true) {
            std::string message;
            std::getline(std::cin, message);

            if (!WriteFile(pipe, message.c_str(), message.length(), NULL, NULL)) {
                std::cerr << "WriteFile failed, GLE=" << GetLastError() << std::endl;
                break;
            }

            if (message == "exit") {
                break;
            }
        }

        serverThread.join();

        DisconnectNamedPipe(pipe);
        CloseHandle(pipe);
        CloseHandle(event);
    }
    else if (choice == 2) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return;
        }

     
        SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return;
        }


        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(DEFAULT_PORT);

       
        if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        std::cout << "Server waiting for client..." << std::endl;

 
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        std::cout << "Client connected. Start chatting!" << std::endl;

    std::thread serverThread(ServerThreadOnSockets, clientSocket);

  
        while (true) {
            std::string message;
            std::getline(std::cin, message);

            if (send(clientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
                break;
            }

            if (message == "exit") {
                break;
            }
        }

  
        serverThread.join();
        
        closesocket(clientSocket);
        closesocket(listenSocket);
        WSACleanup();
    }
    else if (choice == 3) {
        HANDLE hMailSlot = CreateMailslot(mailslotName, 0, MAILSLOT_NO_MESSAGE, nullptr);

        if (hMailSlot == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_ALREADY_EXISTS) {
                std::cerr << "Mailslot already exists." << std::endl;
            }
            else {
                std::cerr << "Failed to open mailslot. Error code: " << GetLastError() << std::endl;
            }
            return;
        }

        // Запускаємо окремий потік для обробки мейлслота
        std::thread serverThread(ServerThreadOnMailslot, hMailSlot);

        // Потік для відправлення повідомлень клієнту
        std::thread clientThread(ClientThreadOnMailslot, mailslotName);

        serverThread.join();
        clientThread.join();

        CloseHandle(hMailSlot);


    }
}


void Client() {
    std::ifstream inputFile("number.txt");
    //ifstream inputFile(filePath);

    int choice;
    if (inputFile.is_open()) {
        inputFile >> choice;
        std::cout << "Server choice: " << types[choice - 1] << std::endl;

        inputFile.close();
    }
    else {
        std::cout << "Unable to open file for reading." << std::endl;
        return; 
    }

    if (choice == 1) {
        HANDLE pipe = CreateFile(
            PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL
        );

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateFile failed, GLE=" << GetLastError() << std::endl;
            return;
        }

        std::cout << "Connected to server. Start chatting!" << std::endl;

        
        std::thread clientThread(ClientThreadOnPipes, pipe);

        while (true) {
            std::string message;
            std::getline(std::cin, message);

            if (!WriteFile(pipe, message.c_str(), message.length(), NULL, NULL)) {
                std::cerr << "WriteFile failed, GLE=" << GetLastError() << std::endl;
                break;
            }

            if (message == "exit") {
                break;
            }
        }

        clientThread.join();

        CloseHandle(pipe);
    }
    else if (choice == 2) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup �� �������." << std::endl;
            return;
        }

       
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed : " << WSAGetLastError() << std::endl;
            WSACleanup();
            return;
        }

     
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(DEFAULT_PORT);

    
        if (InetPton(AF_INET, _T("127.0.0.1"), &serverAddr.sin_addr) != 1) {
            std::cerr << "Incorrect conection addres." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return;
        }

       
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return;
        }

       
        std::cout << "Connected to server. Start chatting!" << std::endl;

       
        std::thread clientThread(ClientThreadOnSockets, clientSocket);

      
        while (true) {
            std::string message;
            std::getline(std::cin, message);

            
            if (send(clientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                std::cerr << "WriteFile failed: " << WSAGetLastError() << std::endl;
                break;
            }

            if (message == "exit") {
                break;
            }
        }

        clientThread.join();
        
        closesocket(clientSocket);
        WSACleanup();
    }
    else if (choice == 3) {

        std::thread clientThread(ClientThreadOnMailslot, mailslotName);
        clientThread.join();
    }
    else {
        std::cerr << "Invalid choice." << std::endl;
    }

}

int main() {
    int choice;
    std::cout << "Choose role (1 - Server, 2 - Client): ";
    std::cin >> choice;

    if (choice == 1) {
        Server();
    }
    else if (choice == 2) {
        Client();
    }
    else {
        std::cerr << "Invalid choice." << std::endl;
        return 1;
    }
    return 0;
}