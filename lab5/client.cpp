#include <iostream>
#include "windows.h"
#include "defs.h"
#include "employee.h"
#include "sstream"

HANDLE startAllEventHandle;
HANDLE readyEventsHandle;
HANDLE pipeHandle;

bool prepareWin32(int processID){
    startAllEventHandle = OpenEvent(SYNCHRONIZE, FALSE, startAllEventName.c_str());
    readyEventsHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, &readyEventName.str()[0] + processID);
    if (startAllEventHandle == NULL)
    {
        std::cerr << "startAllOpenEvent failed!";
        return false;
    }
    if(readyEventsHandle == NULL){
        std::cerr << "readyEvent failed!";
        return false;
    }
    return true;
}

bool connect(){
        while (true)
        {
            pipeHandle = CreateFile(
                    pipeName.c_str(),
                    GENERIC_READ |
                    GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

            if (pipeHandle != INVALID_HANDLE_VALUE) {
                std::cout << "ivalid handle value";
                break;
            }

            if (GetLastError() != ERROR_PIPE_BUSY)
            {
                std::cout << "pipe busy";
                return false;
            }

            if ( ! WaitNamedPipe(pipeName.c_str(), 20000))
            {
                printf("Could not open pipe: 20 second wait timed out.");
                return false;
            }
        }
        return true;
    }

void processingFunc()
{
    bool readSuccess = false;
    bool sendSuccess = false;
    employee emp;

    while (true)
    {
        DWORD cbRead;
        DWORD cbWritten;

        int selection;
        bool selectionOK = false;
        do
        {
            std::cout << "What to do?" << std::endl;
            std::cout << "1. Read entry" << std::endl;
            std::cout << "2. Modify entry" << std::endl;
            std::cout << "3. Exit" << std::endl;
            std::cout << "Your selection : ";
            std::cin >> selection;
            if (selection < 1 || selection > 3) std::cout << "Wrong selection! Try again." << std::endl;
            else selectionOK = true;
        } while(!selectionOK);
        if (selection == 3)
        {
            std::cout<<("Bye! o/");
            break;
        }

        std::ostringstream message;
        int id;

        std::cout << "Enter id : ";
        std::cin >> id;

        if (selection == 1) message << "r " << id;
        else message << "w " << id;

        char command[msgMaxCapacity];
        strcpy(command, &message.str()[0]);

        sendSuccess = WriteFile(pipeHandle, command, msgMaxCapacity, &cbRead, NULL);
        if (!sendSuccess)
        {
            std::cout << ("Failed to send the command!");
            break;
        }
        readSuccess = ReadFile(pipeHandle, &emp, sizeof(employee), &cbRead, NULL);
        if (!readSuccess)
        {
            std::cerr << ("Failed to read server's answer");
            break;
        }

        if (emp.getNum() == -1)
        {
            std::cerr << ("The entry doesn't exist or is blocked.");
            continue;
        }

        std::cout << emp;

        if (selection == 1)
        {
            std::cout << ("Press any key to send cancel message to the server.\n");
            system("pause");
            message.str("");
            message.clear();
            message << "c " << id;
            strcpy(command, &message.str()[0]);
            sendSuccess = WriteFile(pipeHandle, command, msgMaxCapacity, &cbWritten, NULL);
            if (!sendSuccess)
            {
                std::cerr << ("Failed to send the command!");
                break;
            }
        }
        if (selection == 2)
        {
            std::cout << ("Enter new info : ");
            std::cin >> emp;
            sendSuccess = WriteFile(pipeHandle, &emp, sizeof(employee), &cbWritten, NULL);
            if (!sendSuccess)
            {
                std::cerr << ("Failed to send the message!");
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int id = std::atoi(argv[0]);
    bool prepareWin32OK = prepareWin32(id);
    SetEvent(readyEventsHandle);
    WaitForSingleObject(startAllEventHandle, INFINITE);
    bool connectOK = connect();
    processingFunc();
    std::cout << "END";
    return 0;
}