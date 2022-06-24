#include <iostream>
#include <fstream>
#include <sstream>
#include <conio.h>
#include "employee.h"
#include "windows.h"

HANDLE readyEventHandler;
HANDLE startAllEventHandler;
HANDLE pipeHandler;

bool openHandles(int processID){
    startAllEventHandler = OpenEvent(SYNCHRONIZE, FALSE, startAllEventName.c_str());
    std::ostringstream handleName;
    handleName << processID;
    readyEventHandler = OpenEvent(EVENT_MODIFY_STATE, FALSE, &handleName.str()[0]);
    if (startAllEventHandler == NULL || readyEventHandler == NULL){
        std::cerr << "ERROR : can't open event\n";
        return false;
    }
    return true;
}

bool connect(){
    while (true){
        pipeHandler = CreateFile(
                pipeName.c_str(),
                GENERIC_READ |
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
        if (pipeHandler != INVALID_HANDLE_VALUE)
            break;
        if (GetLastError() != ERROR_PIPE_BUSY){
            std::cerr << "ERROR: can't open pipe\n";
            return false;
        }
        if ( ! WaitNamedPipe(pipeName.c_str(), 20000)){
            std::cout << "Could not open pipe: 20 second wait timed out";
            return false;
        }
    }
    return true;
}

void processingFunc(){
    bool readSuccess = false;
    bool sendSuccess = false;
    employee emp;

    while (true){
        DWORD cbRead;
        DWORD cbWritten;
        int selection;
        bool selectionOK = false;
        do{
            std::cout << "What to do?" << '\n';
            std::cout << "1. Read entry" << '\n';
            std::cout << "2. Modify entry" << '\n';
            std::cout << "3. Exit" << '\n';
            std::cout << "Your selection : ";
            std::cin >> selection;
            if (selection < 1 || selection > 3) std::cout << "Wrong selection! Try again." << '\n';
            else selectionOK = true;
        }
        while(!selectionOK);
        if (selection == 3){
            break;
        }
        std::ostringstream message;
        int id;
        std::cout << "Enter id : ";
        std::cin >> id;
        if (selection == 1)
            message << "r " << id;
        else
            message << "w " << id;
        char command[MESSAGE_MAX_SIZE];
        strcpy(command, &message.str()[0]);
        if(!WriteFile(pipeHandler, command, MESSAGE_MAX_SIZE,
                      &cbRead, NULL)){
            std::cerr << "ERROR: can't sent command\n";
            break;
        }
        if(!ReadFile(pipeHandler, &emp, sizeof(employee),
                     &cbRead, NULL)){
            std::cerr << "ERROR: failed to read message\n";
            break;
        }
        if (emp.getId() == -1){
            std::cerr << "The entry doesn't exist or is blocked\n";
            continue;
        }
        std::cout << emp;
        if (selection == 1){
            message.str("");
            message.clear();
            message << "c " << id;
            strcpy(command, &message.str()[0]);
            sendSuccess = WriteFile(pipeHandler, command, MESSAGE_MAX_SIZE, &cbWritten, NULL);
            if (!sendSuccess){
                std::cerr << "ERROR: failed to sent command\n";
                break;
            }
        }
        if (selection == 2){
           std::cin >> emp;
            sendSuccess = WriteFile(pipeHandler, &emp, sizeof(employee), &cbWritten, NULL);
            if (!sendSuccess){
                std::cerr << "ERROR : can't sent message\n";
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int id = std::atoi(argv[0]);
    if(!openHandles(id)){
        std::cerr << "ERROR: Something went wrong while open handles\n";
        return -1;
    }
    SetEvent(readyEventHandler);
    WaitForSingleObject(startAllEventHandler, INFINITE);
    if(!connect()){
        std::cerr << "ERROR : failed to connect \n";
        return -1;
    }
    else
        std::cout << "I am connected now\n";
    processingFunc();
    return 0;
}