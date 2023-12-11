#pragma once

void ServerThreadOnPipes(HANDLE pipe) {
    char buffer[512];
    DWORD bytesRead;

    while (true) {
        if (!ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            std::cerr << "ReadFile failed, GLE=" << GetLastError() << std::endl;
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Client: " << buffer << std::endl;
    }
}


void ClientThreadOnPipes(HANDLE pipe) {
    char buffer[512];
    DWORD bytesRead;

    while (true) {
        if (!ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            std::cerr << "ReadFile failed, GLE=" << GetLastError() << std::endl;
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }
}
