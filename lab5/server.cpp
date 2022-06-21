#include <iostream>
#include "windows.h"
#include "defs.h"
#include "employee.h"
#include <vector>
#include <fstream>

CRITICAL_SECTION csFileWrite;
CRITICAL_SECTION csBlockedFlags;
HANDLE startAllEventHandle;
std::vector<HANDLE> readyEventsHandles;
std::vector<HANDLE> serverThreadsHandles;
std::vector<EntryState> entryStates;
int empCount;

int findEmployeeInFile(int num){
    std::fstream fs(fileName.c_str(), std::ios::binary | std::ios::in);
    for(int i = 0 ; i < empCount; ++i){
        employee emp;
        fs >> emp;
        if(emp.getNum() == num)
            return i;
    }
    return -1;
}

bool createClient(int clientID)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    std::ostringstream args;
    args << clientID;

    bool createOk = CreateProcess( clientExeName.c_str(),
                                   &args.str()[0],
                                   NULL,
                                   NULL,
                                   FALSE,
                                   CREATE_NEW_CONSOLE,
                                   NULL,
                                   NULL,
                                   &si,
                                   &pi
    );
    if (!createOk)
    {
      std::cerr << ("CreateProcess failed!");
    }

    CloseHandle(pi.hThread);
    return createOk;
}

void createBinary(){
    std::fstream fout(fileName.c_str(), std::ios::binary | std::ios::out);
    std::cout << "enter employee count\n";
    std::cin >> empCount;
    for(int i = 0; i < empCount; ++i){
        std::cout << "enter employee id, name and working hours\n";
        employee emp;
        std::cin >> emp;
        fout << emp;
    }
}


void printBinaryFile()
{
    employee emp;
    std::fstream fs(fileName.c_str(), std::ios::binary | std::ios::in);
    for (int i = 0; i < empCount; ++i)
    {
        fs >> emp;
        std::cout << emp << '\n';
    }
}


DWORD WINAPI startMessageConnection(LPVOID params){
    HANDLE hPipe = params;
    bool readSuccess = false;
    bool sendSuccess = false;
    char message[msgMaxCapacity];
    int id;
    employee emp;
    while (true)
    {
        DWORD cbRead;
        DWORD cbWritten;
        bool allowModify = false;

        readSuccess = ReadFile(hPipe, message, msgMaxCapacity, &cbRead, NULL);
        if (!readSuccess)
        {
            std::cerr << ("Failed to read the message!");
            break;
        }

        id = std::atoi(message + 2);
        EnterCriticalSection(&csFileWrite);
        int posInFile = findEmployeeInFile(id);
        if (posInFile != -1)
        {
            std::fstream fs(fileName.c_str(), std::ios::binary | std::ios::in);
            fs.seekg(posInFile * sizeof(employee));
            fs >> emp;
        }
        else emp.setNum(-1);
        LeaveCriticalSection(&csFileWrite);
        EnterCriticalSection(&csBlockedFlags);
        switch (message[0])
        {
            case 'r':
            {
                if (posInFile != -1) entryStates[posInFile] = IS_BEING_READ;
                break;
            }
            case 'w':
            {
                if(posInFile != -1)
                {
                    if (entryStates[posInFile] == IS_BEING_READ) emp.clear();
                    else
                    {
                        entryStates[posInFile] = IS_BEING_MODIFIED;
                        allowModify = true;
                    }
                }
                break;
            }
            case 'c':
            {
                if (posInFile != -1) entryStates[posInFile] = IS_FREE;
                break;
            }
        }
        LeaveCriticalSection(&csBlockedFlags);
        if (message[0] == 'c') continue;
        sendSuccess = WriteFile(hPipe, &emp, sizeof(employee), &cbWritten, NULL);
        if (!sendSuccess)
        {
            std::cerr << ("Failed to send the message!");
            break;
        }

        if (message[0] == 'w' && allowModify)
        {
            readSuccess = ReadFile(hPipe, &emp, sizeof(employee), &cbRead, NULL);
            if (!readSuccess)
                {
                    break;
            }
            if (posInFile != - 1)
            {
                EnterCriticalSection(&csFileWrite);
                std::fstream fs(fileName.c_str(), std::ios::binary | std::ios::out);
                fs.seekp(posInFile * sizeof(employee));
                fs << emp;
                fs.close();
                LeaveCriticalSection(&csFileWrite);
                EnterCriticalSection(&csBlockedFlags);
                entryStates[posInFile] = IS_FREE;
                LeaveCriticalSection(&csBlockedFlags);
            }
        }
    }
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

bool createPipes(int clientsCount) {
    serverThreadsHandles = std::vector<HANDLE>(clientsCount);
    HANDLE hPipe;
    for (int i = 0; i < clientsCount; ++i)
    {
        hPipe = CreateNamedPipe(
                pipeName.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE |
                PIPE_READMODE_MESSAGE |
                PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                0,
                0,
                0,
                NULL);

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            std::cerr << ("CreateNamedPipe failed!");
            return false;
        }

        bool isConnected = ConnectNamedPipe(hPipe, NULL) != 0 || (GetLastError() == ERROR_PIPE_CONNECTED);
        if (isConnected)
        {
            serverThreadsHandles[i] = CreateThread(NULL, 0, startMessageConnection, (LPVOID) hPipe, 0, NULL);
            if (serverThreadsHandles[i] == NULL)
            {
                std::cerr << ("CreateThread for server failed!");
                return false;
            }
        }
        else
        {
            CloseHandle(hPipe);
        }
    }
    return true;
}

bool initializeHandles(int clientsCount){
    InitializeCriticalSection(&csBlockedFlags);
    InitializeCriticalSection(&csFileWrite);
    startAllEventHandle = CreateEvent(NULL, TRUE, FALSE, startAllEventName.c_str());
    entryStates = std::vector<EntryState>(clientsCount);
    if (startAllEventHandle == NULL)
    {
        std::cerr << ("CreateEvent failed!");
        return false;
    }
    readyEventsHandles = std::vector<HANDLE>(clientsCount);
        for (int i = 0; i < clientsCount; ++i) {
            readyEventsHandles[i] = CreateEvent(NULL, TRUE, FALSE, &readyEventName.str()[0] + i);
            createClient(i);
            if (readyEventsHandles[i] == NULL) {
                std::cerr << "create event failure";
                return false;
            }
        }
    return true;
}


int main() {
    createBinary();
    std::cout << "enter clients count\n";
    int clientsCount;
    std::cin >> clientsCount;
    initializeHandles(clientsCount);
    WaitForMultipleObjects(clientsCount, readyEventsHandles.data(), TRUE, INFINITE);
    SetEvent(startAllEventHandle);
    createPipes(clientsCount);
    WaitForMultipleObjects(clientsCount, serverThreadsHandles.data(), TRUE, INFINITE);
    printBinaryFile();
    return 0;
}