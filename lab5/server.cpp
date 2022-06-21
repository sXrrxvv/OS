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
std::vector<optionType> entryStates;
int empCount;

int findEmployeeInFile(int num){
    std::fstream file;
    file.open(fileName.c_str(), std::ios::binary);
    for(int i = 0 ; i < empCount; ++i){
        employee emp;
        file >> emp;
        if(emp.getNum() == i)
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
    return CreateProcess( clientExeName.c_str(),
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

void printBinaryFile(){
    std::fstream file(fileName.c_str(), std::ios::binary);
    for(int i = 0; i < empCount; ++i){
        employee emp;
        file >> emp;
        std::cout << emp << '\n';
    }
}


DWORD WINAPI startMessageConnection(LPVOID params){
    HANDLE hPipe = params;
    bool readSuccess = false;
    bool sendSuccess = false;
    char msg[msgMaxCapacity];
    int id;
    employee emp;
    while (true)
    {
        DWORD cbRead;
        DWORD cbWritten;
        bool allowModify = false;
        if(!ReadFile(hPipe, msg, msgMaxCapacity, &cbRead, NULL)){
            std::cerr << "read from client failure";
            break;
        }
        id = std::atoi(msg + 2);
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
        switch (msg[0])
        {
            case 'r':
            {
                if (posInFile != -1) entryStates[posInFile] = read;
                break;
            }
            case 'w':
            {
                if(posInFile != -1)
                {
                    if (entryStates[posInFile] == read)
                        emp.clear();
                    else
                    {
                        entryStates[posInFile] = modification;
                        allowModify = true;
                    }
                }
                break;
            }
            case 'c':
            {
                if (posInFile != -1)
                    entryStates[posInFile] = unused;
                break;
            }
        }
        LeaveCriticalSection(&csBlockedFlags);
        if (msg[0] == 'c') continue;
        sendSuccess = WriteFile(hPipe, &emp, sizeof(employee), &cbWritten, NULL);
        if (msg[0] == 'w' && allowModify)
        {
            readSuccess = ReadFile(hPipe, &emp, sizeof(employee), &cbRead, NULL);
            if (posInFile != - 1)
            {
                EnterCriticalSection(&csFileWrite);
                std::fstream fs(fileName.c_str(), std::ios::binary);
                fs.seekp(posInFile * sizeof(employee));
                fs << emp;
                fs.close();
                LeaveCriticalSection(&csFileWrite);
                EnterCriticalSection(&csBlockedFlags);
                entryStates[posInFile] = unused;
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
    HANDLE pipeHandle;
    for (int i = 0; i < clientsCount; ++i) {
        pipeHandle = CreateNamedPipe(
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
        if (pipeHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipe failed!";
            return false;
        }
        if (!ConnectNamedPipe(pipeHandle, NULL)) {
            CloseHandle(pipeHandle);
            return false;
        }
        serverThreadsHandles[i] = CreateThread(NULL, 0,
                                               startMessageConnection, static_cast<LPVOID> (pipeHandle), 0, NULL);
        if (serverThreadsHandles[i] == NULL) {
            std::cerr << "CreateThread for server failed";
            return false;
        }
    }
    return true;
}

bool initializeHandles(int clientsCount){
    InitializeCriticalSection(&csBlockedFlags);
    InitializeCriticalSection(&csFileWrite);
    startAllEventHandle = CreateEvent(NULL, TRUE, FALSE, startAllEventName.c_str());
    entryStates = std::vector<optionType>(clientsCount);
    if (startAllEventHandle == NULL)
    {
        std::cerr << "CreateEvent failed!";
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