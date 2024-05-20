#ifndef IHTTPSERVERH
#define IHTTPSERVERH
#include <WinSock2.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <fstream>
#include <random>
#include "ServerSettings.h"
#include "../lib/HTTPrequest.h"
#include "../lib/HTTPresponse.h"
#include "../headers/HttpDate.h"

class IHTTPServer{
public:
    virtual std::shared_ptr<HTTPrequest> readRequest(SOCKET s) = 0;
    virtual void sendResponse(SOCKET s,const HTTPresponse& response) = 0;
    virtual void work(SOCKET s,std::shared_ptr<HTTPrequest> req,in_addr ip) = 0;
    virtual std::shared_ptr<HTTPresponse> makeErrorResponse(uint32_t status_code,const std::string& status,bool isHead = false)const = 0;
    virtual const ServerSettings& getSettings() const = 0;
    virtual void setSettings(ServerSettings & settings) = 0;
    virtual HANDLE getMutexConsole() = 0;
    virtual HANDLE getMutexLog() = 0;
    virtual void WriteTolog(const std::wstring str) = 0;
};
#endif