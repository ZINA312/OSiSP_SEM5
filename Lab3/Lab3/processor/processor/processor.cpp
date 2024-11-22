#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

const char* pipeName = "\\\\.\\pipe\\my_pipe";



void processRequest(const std::string& data) {
    std::cout << "Обрабатывается: " << data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); 
}

void readFromPipe(HANDLE hPipe) {
    char buffer[128];
    DWORD bytesRead;

    while (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0'; 
        std::string data(buffer);
        std::thread(processRequest, data).detach();
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    HANDLE hPipe = CreateFileA(
        pipeName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия канала." << GetLastError() << std::endl;
        return 1;
    }

    readFromPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}