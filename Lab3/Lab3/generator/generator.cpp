#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

const char* pipeName = "\\\\.\\pipe\\my_pipe";

void processRequest(const std::string& data) {
    std::cout << "Обрабатывается: " << data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); 
}

int main() {
    setlocale(LC_ALL, "Russian");
    HANDLE hPipe = CreateNamedPipeA(
        pipeName,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024,
        1024,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка создания канала." << std::endl;
        return 1;
    }

    std::cout << "Ожидание подключения..." << std::endl;
    ConnectNamedPipe(hPipe, NULL);

    for (int i = 0; i < 10; ++i) {
        std::string data = "Заявка " + std::to_string(i);
        DWORD written;
        WriteFile(hPipe, data.c_str(), data.size() + 1, &written, NULL);
        std::cout << "Сгенерировано: " << data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CloseHandle(hPipe);
    return 0;
}