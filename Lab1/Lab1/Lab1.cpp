#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <iostream>

struct ProcessInfo {
    DWORD processID;
    std::wstring processName;
    PROCESS_INFORMATION processInfo;
    ProcessInfo(DWORD id, const std::wstring& name, const PROCESS_INFORMATION& info)
        : processID(id), processName(name), processInfo(info) {}
};
DWORD targetProcessID;
std::vector<ProcessInfo> processes;

void LaunchProcess(const std::wstring& filePath) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(filePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        Sleep(1000);
        processes.push_back({ pi.dwProcessId, filePath, pi });
        std::cout << "Процесс запущен!\n\n";
    }
    else {
        MessageBox(NULL, L"Не удалось запустить процесс", L"Ошибка", MB_OK | MB_ICONERROR);
    }

}

BOOL CALLBACK CloseProcess(HWND hwnd, LPARAM lParam) {
    DWORD processID;
    GetWindowThreadProcessId(hwnd, &processID);
    if (processID == targetProcessID) {
        LRESULT res = ::SendMessage(hwnd, WM_CLOSE, NULL, NULL);
        return FALSE; 
    }
    return TRUE;
}

void EnumProcesses() {
    for (const auto& proc : processes) {
        DWORD waitResult = WaitForSingleObject(proc.processInfo.hProcess, 0);
        if (waitResult == WAIT_TIMEOUT) {
            std::wcout << L"Процесс: " << proc.processName
                << L" (ID: " << proc.processID << "): Running" << std::endl;
        }
        else {
            std::wcout << L"Процесс: " << proc.processName
                << L" (ID: " << proc.processID << "): Closed" << std::endl;
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    while (true) {
        std::cout << "1. Запустить процесс\n";
        std::cout << "2. Закрыть процесс\n";
        std::cout << "3. Показать состояния процессов\n";
        std::cout << "4. Выход\n";
        int choice;
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cout << "Неверный ввод!\n";
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
        }
        else if (choice == 1) {
            std::cout << "Введите путь к исполняемому файлу: ";
            std::wstring filePath;
            std::wcin >> filePath;
            LaunchProcess(filePath);
        }
        else if (choice == 2) {
            std::cout << "Введите ID процесса для закрытия: ";
            std::cin >> targetProcessID;
            EnumWindows(CloseProcess, 0);
        }
        else if (choice == 3) {
            EnumProcesses();
        }
        else if (choice == 4) {
            break;
        }
        else {
            std::cout << "Нет такого варианта!\n";
        }
    }

    for (auto& proc : processes) {
        CloseHandle(proc.processInfo.hProcess);
        CloseHandle(proc.processInfo.hThread);
    }

    return 0;
}