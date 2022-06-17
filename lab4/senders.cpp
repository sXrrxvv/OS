#include <iostream>
#include <sstream>
#include <fstream>
#include "windows.h"
#include "definitions.h"
#include "message.h"

HANDLE maxAmountMsgSem;
HANDLE readMessageSemHandler;
HANDLE sharedFileMutexHandler;

HANDLE readyEventHandler;

void openHandlers(uInt id)
{
    sharedFileMutexHandler = OpenMutex(MUTEX_ALL_ACCESS, FALSE, fileMutexName.c_str());
    readMessageSemHandler = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, readSemName.c_str());
    maxAmountMsgSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, maxAmountSemName.c_str());
    readyEventHandler = OpenEvent(EVENT_MODIFY_STATE, FALSE, &eventName.str()[0] + id);
}

void cleanAll()
{
    CloseHandle(readyEventHandler);
    CloseHandle(sharedFileMutexHandler);
    CloseHandle(maxAmountMsgSem);
    CloseHandle(readMessageSemHandler);
}

void sender(std::string& binaryFileName)
{
    std::fstream file;
    while(true)
    {
        int option;
        bool validOption = false;
        while(!validOption) {
            std::cout << "Options" << '\n' << "type 3 for writing message" << '\n'
                      << "type 2 for exiting receiver" << '\n';
            std::cin >> option;
            if (option != writeMessage && option != stop) {
                std::cout << "Unrecognised command, try again" << '\n';
            }
            else
                validOption = true;
        }
        WaitForSingleObject(maxAmountMsgSem, INFINITE);
        message messageToSend;
        std::cout << "Enter message to send : " << '\n';
        std::cin >> messageToSend;
        WaitForSingleObject(sharedFileMutexHandler, INFINITE);
        try {
            file.open(binaryFileName.c_str(), std::ios::binary | std::ios::out);
        }
        catch (std::ios_base::failure&){
            std::cerr << "cant open file\n";
            return;
        }
        file << messageToSend;
        file.close();
        ReleaseMutex(sharedFileMutexHandler);
        ReleaseSemaphore(readMessageSemHandler, 1, NULL);
    }
}

int main(int argc, char *argv[])
{
    uInt id = std::atoi(argv[0]);
    std::string binaryFileName = argv[1];
    openHandlers(id);
    SetEvent(readyEventHandler);
    sender(binaryFileName);
    cleanAll();
    return 0;
}
