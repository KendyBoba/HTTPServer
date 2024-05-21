#ifndef SETTINGSH
#define SETTINGSH
#include <fstream>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include "winsock.h"
#include "../lib/OtherFunction.h"
#include "../lib/HTTPrequest.h"

class ServerSettings{
private:
    struct access_info{
        std::wstring path = L"";
        std::string ip = "";
        std::string method = "";
        bool isAllow = false;
    };
private:
    std::unique_ptr<std::unordered_map<std::string,std::string>> mime_types;
    std::unique_ptr<std::vector<access_info>> access_resource;
    std::unique_ptr<std::map<std::string,std::wstring>> association_parser;
    std::wstring main_dir = L"projects";
    uint16_t max_threads = 1000;
    uint16_t max_conn = 1000;
    uint16_t max_time_wait_sec = 10;
    uint16_t port = 80;
    unsigned long ip = INADDR_ANY;
    std::wstring default_file = L"index.html";
    std::string default_postfix = ".dll";
    std::wstring default_handler = L"";
public:
    ServerSettings();
    ServerSettings(const std::wstring& mime_path,const std::wstring& access_path,const std::wstring& settings_path);
    ServerSettings(const ServerSettings& settings);
    ServerSettings(ServerSettings&& settings);
    ServerSettings& operator=(const ServerSettings& settings);
    ServerSettings& operator=(ServerSettings&& settings);
    void ReadMimeTypes(const std::wstring& path);
    void ReadAccesMethod(const std::wstring& path);
    void ReadSettings(const std::wstring& path);
    const std::unordered_map<std::string,std::string>& getMimeTypes()const;
    const std::map<std::string,std::wstring>& getAssociation()const;
    bool isMethodAccess(const std::wstring& path,METHOD method,const std::string& ip)const;
    const std::wstring& getMainDir()const;
    const std::wstring& getDefaultResorce()const;
    const std::string& getDefaultPostfix()const;
    const std::wstring& getDefaultHandlerFile()const;
    uint16_t getMaxThreads()const;
    uint16_t getMaxConnections()const;
    uint16_t getTimeWait()const;
    uint16_t getPort()const;
    unsigned long getIp()const;
private:
    bool compare(const std::string & text,const std::string &mask)const;
    bool compare(const std::wstring & text,const std::wstring &mask)const;
};

#endif