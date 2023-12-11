#pragma once

void ClientThreadOnMailslot(const wchar_t* mailslotName) {
    HANDLE hMailSlot = CreateFile(mailslotName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hMailSlot == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFile failed, GLE=" << GetLastError() << std::endl;
        return;
    }

    while (true) {
        std::string message;
        std::getline(std::cin, message);

        if (WriteFile(hMailSlot, message.c_str(), message.length(), NULL, NULL)) {
            if (message == "exit") {
                break;
            }
        }
        else {
            std::cerr << "WriteFile failed, GLE=" << GetLastError() << std::endl;
            break;
        }
    }

    CloseHandle(hMailSlot);
}

void ServerThreadOnMailslot(HANDLE hMailSlot) {
    char buffer[512];
    DWORD bytesRead;

    while (true) {
        bytesRead = 0;
        if (ReadFile(hMailSlot, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Received message: " << buffer << std::endl;
        }
    }
}


