#pragma once


void ClientThreadOnSockets(SOCKET serverSocket) {
    char buffer[512];
    int bytesRead;

    
    while (true) {
        bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Server disconected." << std::endl;
            closesocket(serverSocket);
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }
}

void ServerThreadOnSockets(SOCKET clientSocket) {
    char buffer[512];
    int bytesRead;

   
    while (true) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Client disconnected." << std::endl;
            closesocket(clientSocket);
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Client: " << buffer << std::endl;
    }
}
