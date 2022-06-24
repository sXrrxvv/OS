#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "employee.h"
#include "windows.h"

int employeesCount;
int clientsCount;
std::string binaryFileName;

CRITICAL_SECTION fileWriteCritSection;
CRITICAL_SECTION blockedFlagsCritSection;

HANDLE startAllEventHandler;

std::vector<HANDLE> readyEventHandlers;
std::vector<HANDLE> serverThreadsHandlers;
std::vector<EntryState> entryStatesHandlers;

void prepareBinaryFile(){
    std::fstream fout(binaryFileName.c_str(), std::ios::binary | std::ios::out);
    for (int i = 0; i < employeesCount; ++i){
        employee emp;
        std::cout << "enter employee id, name and working hours\n";
        std::cin >> emp;
        fout << emp;
    }
}

void printBinaryFile(){
    employee emp;
    std::fstream fs(binaryFileName.c_str(), std::ios::binary | std::ios::in);
    for (int i = 0; i < employeesCount; ++i){
        fs >> emp;
        std::cout << emp << '\n';
    }
}

int findEmployeeInFile(int id){
    std::fstream fs(binaryFileName.c_str(), std::ios::binary | std::ios::in);
    employee emp;
    for (int i = 0; i < employeesCount; ++i){
        fs >> emp;
        if (emp.getId() == id){
            return i;
        }
    }
    return notExistIndex;
}

bool createClient(int clientID){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    std::ostringstream args;
    args << clientID;
    if(!CreateProcess( clientExeName.c_str(),
                                   &args.str()[0],
                                   NULL,
                                   NULL,
                                   FALSE,
                                   CREATE_NEW_CONSOLE,
                                   NULL,
                                   NULL,
                                   &si,
                                   &pi
    )){
        std::cerr << "CreateProcess failed!";
        return false;
    }
    CloseHandle(pi.hThread);
    return true;
}

bool prepareSyncObjects(){
    InitializeCriticalSection(&blockedFlagsCritSection);
    InitializeCriticalSection(&fileWriteCritSection);
    startAllEventHandler = CreateEvent(NULL, TRUE, FALSE, startAllEventName.c_str());
    entryStatesHandlers = std::vector<EntryState>(clientsCount);
    if (startAllEventHandler == NULL){
        std::cerr << "ERROR: can't create event";
        return false;
    }
    return true;
}

bool createHandles(){
    bool syncObjPrepared = prepareSyncObjects();
    bool clientsPrepared = true;
    readyEventHandlers = std::vector<HANDLE>(clientsCount);
    std::ostringstream handleName;
    for (int i = 0; i < clientsCount; ++i){
        handleName  << i;
        readyEventHandlers[i] = CreateEvent(NULL, TRUE, FALSE, &handleName.str()[0]);
        clientsPrepared &= createClient(i);
        handleName.str("");
        handleName.clear();
    }
    return syncObjPrepared && clientsPrepared;
}

void cleanAll(){
    for (int i = 0; i < clientsCount; ++i){
        CloseHandle(readyEventHandlers[i]);
    }
    CloseHandle(startAllEventHandler);
    DeleteCriticalSection(&fileWriteCritSection);
    DeleteCriticalSection(&blockedFlagsCritSection);
}

DWORD WINAPI processingThread(LPVOID params)
{
    HANDLE pipeHandler = params;
    char message[MESSAGE_MAX_SIZE];
    int id;
    employee emp;
    while (true)
    {
        DWORD cbRead;
        DWORD cbWritten;
        bool allowModify = false;
        if(!ReadFile(pipeHandler, message, MESSAGE_MAX_SIZE, &cbRead, NULL)){
            std::cerr << "ERROR: can't read message";
            break;
        }
        id = std::atoi(message + offset);
        EnterCriticalSection(&fileWriteCritSection);
        int posInFile = findEmployeeInFile(id);
        if (posInFile != notExistIndex){
            std::fstream fs(binaryFileName.c_str(), std::ios::binary | std::ios::in);
            fs.seekg(posInFile * sizeof(employee));
            fs >> emp;
        }
        else emp.setId(notExistIndex);
        LeaveCriticalSection(&fileWriteCritSection);
        EnterCriticalSection(&blockedFlagsCritSection);
        switch (message[0])
        {
            case 'r':
            {
                if (posInFile != -1) entryStatesHandlers[posInFile] = IS_BEING_READ;
                break;
            }
            case 'w':
            {
                if(posInFile != -1)
                {
                    if (entryStatesHandlers[posInFile] == IS_BEING_READ) emp.clear();
                    else
                    {
                        entryStatesHandlers[posInFile] = IS_BEING_MODIFIED;
                        allowModify = true;
                    }
                }
                break;
            }
            case 'c':
            {
                if (posInFile != -1) entryStatesHandlers[posInFile] = IS_FREE;
                break;
            }
        }
        LeaveCriticalSection(&blockedFlagsCritSection);
        if (message[0] == 'c') continue;
         if(!WriteFile(pipeHandler, &emp, sizeof(employee),
                       &cbWritten, NULL)){
            std::cerr << "ERROR: can't write message";
            break;
        }

        if (message[0] == 'w' && allowModify)
        {
            if(!ReadFile(pipeHandler, &emp, sizeof(employee), &cbRead, NULL));
                std::cerr << "ERROR: can't read client answer";
                break;
            }
                EnterCriticalSection(&fileWriteCritSection);
                std::fstream fs(binaryFileName.c_str(), std::ios::binary | std::ios::out);
                fs.seekp(posInFile * sizeof(employee));
                fs << emp;
                fs.close();
                LeaveCriticalSection(&fileWriteCritSection);
                EnterCriticalSection(&blockedFlagsCritSection);
                entryStatesHandlers[posInFile] = IS_FREE;
                LeaveCriticalSection(&blockedFlagsCritSection);
        }
    FlushFileBuffers(pipeHandler);
    DisconnectNamedPipe(pipeHandler);
    CloseHandle(pipeHandler);
}

void initialize(){

    std::cout << "Enter the number of employees : ";
    std::cin >> employeesCount;
    std::cout << "Enter the number of clients : ";
    std::cin >> clientsCount;
    std::cout << "Enter binary file's name : ";
    std::cin >> binaryFileName;
}

bool makeConnection(){
    serverThreadsHandlers = std::vector<HANDLE>(clientsCount);
    HANDLE pipeHandler;
    for (int i = 0; i < clientsCount; ++i){
        pipeHandler = CreateNamedPipe(
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

        if (pipeHandler == INVALID_HANDLE_VALUE){
            std::cerr << "ERROR: can't create pipe";
            return false;
        }
        bool isConnected = ConnectNamedPipe(pipeHandler, NULL) != 0
                || (GetLastError() == ERROR_PIPE_CONNECTED);
        if (isConnected){
            serverThreadsHandlers[i] = CreateThread(NULL, 0, processingThread,
                                             (LPVOID) pipeHandler, 0, NULL);
            if (serverThreadsHandlers[i] == NULL){
                std::cerr << "ERROR:: can't connect to pipe";
                return false;
            }
        }
        else{
            CloseHandle(pipeHandler);
        }
    }
    return true;
}

int main()
{
    initialize();
    prepareBinaryFile();
    if(!createHandles()){
        std::cerr << "Something went wrong while preparing Win32 stuff!";
        return -1;
    }
    WaitForMultipleObjects(clientsCount, readyEventHandlers.data(), TRUE, INFINITE);
    SetEvent(startAllEventHandler);
    if(!makeConnection()){
        std::cerr << "Something went wrong while establishing server-client connections!\n";
        return -1;
    }
    WaitForMultipleObjects(clientsCount, serverThreadsHandlers.data(), TRUE, INFINITE);
    std::cout << "Final file : \n";
    printBinaryFile();
    cleanAll();
    std::cout << "Bye!\n";
    return 0;
}