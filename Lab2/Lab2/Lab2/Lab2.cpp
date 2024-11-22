#include <iostream>
#include <windows.h>
#include <vector>
#include <string>

const std::wstring FILENAME = L"database.dat";
const int MAX_RECORDS = 200;

struct Record {
    int id;
    wchar_t name[50];
    bool isDeleted; 
};

HANDLE hFile;
HANDLE hMapFile;
Record* pMapView;

void CreateDatabase() {
    hFile = CreateFileW(
        FILENAME.c_str(), 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        NULL, 
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating file: " << GetLastError() << std::endl;
        return;
    }

    DWORD bytesWritten;
    std::vector<Record> records(MAX_RECORDS);
    for (int i = 0; i < MAX_RECORDS; ++i) {
        records[i].isDeleted = true; 
    }

    WriteFile(hFile, records.data(), sizeof(Record) * MAX_RECORDS, &bytesWritten, NULL);
    CloseHandle(hFile);
}

void MapDatabase() {
    hFile = CreateFileW(FILENAME.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file: " << GetLastError() << std::endl;
        return;
    }

    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, sizeof(Record) * MAX_RECORDS, NULL);
    if (hMapFile == NULL) {
        std::cerr << "Could not create file mapping object: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return;
    }

    pMapView = (Record*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Record) * MAX_RECORDS);
    if (pMapView == NULL) {
        std::cerr << "Could not map view of file: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return;
    }
}

void DisplayRecords() {
    std::wcout << L"Существующие записи:\n";
    for (int i = 0; i < MAX_RECORDS; ++i) {
        if (!pMapView[i].isDeleted) {
            std::wcout << L"ID: " << pMapView[i].id << L", Name: " << pMapView[i].name << std::endl;
        }
    }
}

void AddRecord(int id, const wchar_t* name) {
    for (int i = 0; i < MAX_RECORDS; ++i) {
        if (pMapView[i].isDeleted) {
            pMapView[i].id = id;
            wcscpy_s(pMapView[i].name, name);
            pMapView[i].isDeleted = false; 
            std::cout << "Запись добавлена успешно." << std::endl;
            return;
        }
    }
    std::cout << "Нет места для добавления новой записи." << std::endl;
}

void DeleteRecord(int id) {
    for (int i = 0; i < MAX_RECORDS; ++i) {
        if (pMapView[i].id == id && !pMapView[i].isDeleted) {
            pMapView[i].isDeleted = true; 
            std::cout << "Запись удалена успешно." << std::endl;
            return;
        }
    }
    std::cout << "Запись не найдена." << std::endl;
}

void SaveChanges() {
    DWORD bytesWritten;
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, pMapView, sizeof(Record) * MAX_RECORDS, &bytesWritten, NULL);
}

void Cleanup() {
    UnmapViewOfFile(pMapView);
    CloseHandle(hMapFile);
    CloseHandle(hFile);
}

void ShowMenu() {
    std::cout << "Выберите операцию:\n";
    std::cout << "1. Показать записи\n";
    std::cout << "2. Добавить запись\n";
    std::cout << "3. Удалить запись\n";
    std::cout << "4. Сохранить изменения\n";
    std::cout << "5. Выход\n";
}

int main() {
    setlocale(LC_ALL, "Russian");
    CreateDatabase(); 
    MapDatabase();    

    int choice;
    while (true) {
        ShowMenu(); 
        std::cin >> choice;

        switch (choice) {
        case 1:
            DisplayRecords();
            break;
        case 2: {
            int id;
            wchar_t name[50];
            std::cout << "Введите ID новой записи: ";
            std::cin >> id;
            std::wcout << L"Введите имя новой записи: ";
            std::wcin.ignore(); 
            std::wcin.getline(name, 50);
            AddRecord(id, name); 
            break;
        }
        case 3: {
            int id;
            std::cout << "Введите ID записи для удаления: ";
            std::cin >> id;
            DeleteRecord(id); 
            break;
        }
        case 4:
            SaveChanges(); 
            std::cout << "Изменения сохранены." << std::endl;
            break;
        case 5:
            Cleanup(); 
            return 0; 
        default:
            std::cout << "Неверный выбор. Пожалуйста, попробуйте снова." << std::endl;
        }
    }
}