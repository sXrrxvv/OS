#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "windows.h"
#include "definitions.h"
#include "message.h"

HANDLE maxAmountMsgSem;
HANDLE readMessageSemHandler;
HANDLE sharedFileMutexHandler;

std::vector<HANDLE> sendersHandlers;
std::vector<HANDLE> readyEventsHandlers;

HANDLE createSender(const std::string& fileName, uInt senderID){
    std::ostringstream args;
    args << senderID << ' ' << fileName;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    if(!CreateProcess( senderExe.c_str(),
                       &args.str()[0],
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_NEW_CONSOLE,
                       NULL,
                       NULL,
                       &si,
                       &pi
    ))
    {
        std::cout << "Creation Failed " << GetLastError();
    }
    CloseHandle(pi.hThread);
    return pi.hProcess;
}

void createHandlers (uInt msgMaxValue, uInt senderAmount, const std::string& fileName){
    maxAmountMsgSem = CreateSemaphore(NULL, msgMaxValue, msgMaxValue, maxAmountSemName.c_str());
    sharedFileMutexHandler = CreateMutex(NULL, FALSE, fileMutexName.c_str());
    readMessageSemHandler = CreateSemaphore(NULL, 0, msgMaxValue, readSemName.c_str());
    sendersHandlers = std::vector<HANDLE>(senderAmount);
    readyEventsHandlers = std::vector<HANDLE>(senderAmount);
    for(uInt i = 0; i < senderAmount; ++i){
        readyEventsHandlers[i] = CreateEvent(NULL, TRUE, FALSE, &eventName.str()[0] + i);
        sendersHandlers[i] = createSender(fileName, i);
    }
}

void receiver(const std::string& fileName){
    std::fstream sharedFile;
    while (true)
    {
        int option;
        bool validOption = false;
        while(!validOption) {
            std::cout << "Options" << '\n' << "type 1 for reading message" << '\n'
                      << "type 2 for exiting receiver" << '\n';
            std::cin >> option;
            if (option != readMessage && option != stop) {
                std::cout << "Unrecognised command, try again" << '\n';
            }
            else
                validOption = true;
        }
        WaitForSingleObject(readMessageSemHandler, INFINITE);
        WaitForSingleObject(sharedFileMutexHandler, INFINITE);
        try {
            sharedFile.open(fileName.c_str(), std::ios::binary | std::ios::in);
        }
        catch (std::ios_base::failure&){
            std::cerr << "cant open file\n";
        }
        message readMessage;
        sharedFile >> readMessage;
        std::cout << "New message : " << readMessage.getMessage() << '\n';
        sharedFile.close();
        ReleaseMutex(sharedFileMutexHandler);
        ReleaseSemaphore(maxAmountMsgSem, 1, NULL);
    }
}

void clearAll(uInt sendersAmount){
    CloseHandle(sharedFileMutexHandler);
    CloseHandle(maxAmountMsgSem);
    CloseHandle(readMessageSemHandler);
    for(uInt i = 0 ; i < sendersAmount; ++i){
        CloseHandle(sendersHandlers[i]);
        CloseHandle(readyEventsHandlers[i]);
    }
}

int main() {
    std::string fileName;
    uInt sendersAmount;
    uInt maxMsgCount;
    std::fstream sharedFile;
    std::cout << "Enter filename, senders amount and max message count";
    std::cin >> fileName >> sendersAmount >> maxMsgCount;
    try {
        sharedFile.open(fileName.c_str(), std::ios::binary | std::ios::out);
    }
    catch(std::ios_base::failure&){
        std::cerr << "can't open file";
        return -1;
    }
    createHandlers(maxMsgCount, sendersAmount, fileName);
    WaitForMultipleObjects(sendersAmount, readyEventsHandlers.data(), TRUE, INFINITE);
    receiver(fileName);
    clearAll(sendersAmount);
    return 0;
}
