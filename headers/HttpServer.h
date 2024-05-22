#ifndef HTTPSERVERH
#define HTTPSERVERH
#include "IHttpServer.h"

void CALLBACK clientThread(
    PTP_CALLBACK_INSTANCE instance,
    PVOID param,
    PTP_WORK
    );


class HTTPServer : IHTTPServer{
private:
    std::shared_ptr<TP_CALLBACK_ENVIRON> th_pool_env;
    static const unsigned short rwblock_size = 0x400;
    WSADATA wsaData{0};
    SOCKET server_socket = INVALID_SOCKET;
    std::wstring current_dir_path;
    std::wstring current_proj_path;
    std::wstring path_to_log_file;
    ServerSettings settings;
    HANDLE mutex_console = NULL;
    HANDLE mutex_log = NULL;
public:
    HTTPServer() = delete;
    HTTPServer(const ServerSettings& settings);
    ~HTTPServer();
    virtual const ServerSettings& getSettings() const override;
    virtual void setSettings(ServerSettings & settings) override;
    virtual std::shared_ptr<HTTPrequest> readRequest(SOCKET s);
    virtual void sendResponse(SOCKET s,const HTTPresponse& response);
    virtual void work(SOCKET s,std::shared_ptr<HTTPrequest> req,in_addr ip)override;
    virtual HANDLE getMutexConsole()override;
    virtual HANDLE getMutexLog()override;
    virtual void WriteTolog(const std::wstring str)override;
    virtual std::shared_ptr<HTTPresponse> makeErrorResponse(
        uint32_t status_code,
        const std::string& status,
        bool isHead = false) const override;
    void exec();
private:
    void init();
    std::shared_ptr<HTTPresponse> GET(std::shared_ptr<HTTPrequest> req,bool isHead = false)const;
    std::shared_ptr<HTTPresponse> PUT(std::shared_ptr<HTTPrequest> req);
    std::shared_ptr<HTTPresponse> DEL(std::shared_ptr<HTTPrequest> req);
    std::shared_ptr<HTTPresponse> POST(std::shared_ptr<HTTPrequest> req);
    std::shared_ptr<HTTPresponse> LoadDll(std::wstring path,std::shared_ptr<HTTPrequest> req,HMODULE d_lib);
    std::shared_ptr<HTTPresponse> ExternalProcess(std::wstring path_to_exe,const std::wstring path_file,std::shared_ptr<HTTPrequest> req);
    std::shared_ptr<std::string> FileReadAll(const std::wstring & path,size_t off = 0,size_t len = 0)const;
    void FileAppend(const std::wstring & path,const std::string &data);
    void FileWriteAll(const std::wstring & path,const std::string &data);
    size_t getFileSize(const std::wstring &w_path)const;
    std::string FilePostfix(const std::wstring & path)const;
    std::string boundary(const std::vector<std::pair<std::string,std::string>> & data)const;
    bool isFileExist(const std::wstring & path)const;
};

#endif