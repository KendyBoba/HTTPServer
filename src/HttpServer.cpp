#include "../headers/HttpServer.h"

void CALLBACK clientThread(
    PTP_CALLBACK_INSTANCE instance,
    PVOID param, 
    PTP_WORK work
    ){
    std::tuple<IHTTPServer*,SOCKET,in_addr> *p_temp_param = (std::tuple<IHTTPServer*,SOCKET,in_addr>*)param;
    IHTTPServer* serv = nullptr;
    SOCKET client_socket = INVALID_SOCKET;
    in_addr client_ip;
    std::tie(serv,client_socket,client_ip) = *p_temp_param;
    try{
        fd_set read_set{0};
        timeval time_out;
        time_out.tv_sec = serv->getSettings().getTimeWait();
        bool isKeepAlive = false;
        uint64_t max = serv->getSettings().getMaxConnections();
        uint64_t current_iter = 0;
        do{
            FD_ZERO(&read_set);
            FD_SET(client_socket,&read_set);
            int sock_count = select(0,&read_set,NULL,NULL,&time_out);
            if(sock_count == SOCKET_ERROR)
                throw std::runtime_error("select function returned an error");
            if(!sock_count){
                break;
            }
            auto req = serv->readRequest(client_socket);
            serv->work(client_socket,req,client_ip);
            ++current_iter;
            if(current_iter >= max)
                break;
            if(req->Header().Params().count("Connection") && req->Header().Params()["Connection"] == "keep-alive")
                isKeepAlive = true;
            else
                break;
            if(req->Header().Params().count("Keep-Alive")){
                std::string ka = req->Header().Params()["Keep-Alive"];
                auto ka_params = split(ka,{",","="});
                if(ka_params->size() > 1)
                    time_out.tv_sec = std::stoull(ka_params->at(1));
                if(ka_params->size() > 3)
                    max = std::stoull(ka_params->at(3));
            }
        }while(isKeepAlive);
    }catch(std::runtime_error & ex){
        serv->sendResponse(client_socket,*serv->makeErrorResponse(500,"Internal Server Error"));
        std::wstring post_log = L"Response: " + fromUTF8(inet_ntoa(client_ip)) + L" 500 " + L"Internal Server Error";
        serv->WriteTolog(post_log);
    }
    catch(std::invalid_argument & ex){
        serv->sendResponse(client_socket,*serv->makeErrorResponse(400,"Bad Request"));
        std::wstring post_log = L"Response: " + fromUTF8(inet_ntoa(client_ip)) + L" 400 " + L"Bad Request";
        serv->WriteTolog(post_log);
    }
    catch(std::domain_error & ex){
        if(ex.what() == "unexpected method"){
            serv->sendResponse(client_socket,*serv->makeErrorResponse(501,"Not Implemented"));
            std::wstring post_log = L"Response: " + fromUTF8(inet_ntoa(client_ip)) + L" 501 " + L"Not Implemented";
            serv->WriteTolog(post_log);
        }else{
            serv->sendResponse(client_socket,*serv->makeErrorResponse(400,"Bad Request"));
            std::wstring post_log = L"Response: " + fromUTF8(inet_ntoa(client_ip)) + L" 400 " + L"Bad Request";
            serv->WriteTolog(post_log);
        }
    }catch(std::out_of_range & ex){
        serv->sendResponse(client_socket,*serv->makeErrorResponse(400,"Bad Request"));
        std::wstring post_log = L"Response: " + fromUTF8(inet_ntoa(client_ip)) + L" 400 " + L"Bad Request";
        serv->WriteTolog(post_log);
    }
    shutdown(client_socket,SD_BOTH);
    closesocket(client_socket);
    delete p_temp_param;
    CloseThreadpoolWork(work);
    WaitForSingleObject(serv->getMutexConsole(),INFINITE);
    std::cout << "Disconnect " << "ip: " << inet_ntoa(client_ip)
    << " Socket: " << std::to_string(client_socket) 
    << " Date:" << HttpDate::getCurGmtTime().toStr() << std::endl;
    ReleaseMutex(serv->getMutexConsole());
    std::wstring post_log = L"Disconnect " + fromUTF8(inet_ntoa(client_ip));
    serv->WriteTolog(post_log);
}

HTTPServer::HTTPServer(const ServerSettings &settings):
    settings(settings)
{
    init();
}

HTTPServer::~HTTPServer(){
    WSACleanup();
    closesocket(server_socket);
    CloseThreadpoolCleanupGroupMembers(th_pool_env->CleanupGroup,false,NULL);
    CloseThreadpoolCleanupGroup(th_pool_env->CleanupGroup);
    CloseThreadpool(th_pool_env->Pool);
}

void HTTPServer::init()
{
    mutex_console = CreateMutex(NULL,FALSE,NULL);
    if(mutex_console == INVALID_HANDLE_VALUE)
        throw std::runtime_error("console mutex was not created");
    mutex_log = CreateMutex(NULL,FALSE,NULL);
    if(mutex_log == INVALID_HANDLE_VALUE)
        throw std::runtime_error("log mutex was not created");
    wchar_t* current_path = new wchar_t[MAX_PATH];
    GetCurrentDirectory(0x7fff,current_path);
    current_dir_path = current_path;
    delete[] current_path;
    current_path = nullptr;
    current_proj_path = (current_dir_path + L"\\") + settings.getMainDir();
    CreateDirectoryW(current_proj_path.c_str(),NULL);
    path_to_log_file = current_dir_path + L"\\log.txt";

    th_pool_env = std::make_shared<TP_CALLBACK_ENVIRON>();
    InitializeThreadpoolEnvironment(th_pool_env.get());
    PTP_POOL pool = CreateThreadpool(NULL);
    PTP_CLEANUP_GROUP cln_group = CreateThreadpoolCleanupGroup();
    if(!pool || !cln_group)
        throw std::runtime_error("threadpool error code:" + std::to_string(WSAGetLastError()));
    SetThreadpoolCallbackPool(th_pool_env.get(),pool);
    SetThreadpoolCallbackCleanupGroup(th_pool_env.get(),cln_group,NULL);
    SetThreadpoolThreadMinimum(th_pool_env->Pool,1);
    SetThreadpoolThreadMaximum(th_pool_env->Pool,settings.getMaxThreads());
    if(WSAStartup(MAKEWORD(2,2),&wsaData))
        throw std::runtime_error("wsastartup error");
    sockaddr_in server_addr{0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(settings.getPort());
    server_addr.sin_addr.S_un.S_addr = settings.getIp();
    server_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(server_socket == INVALID_SOCKET)
        throw std::runtime_error("server socket was not created");
    if(bind(server_socket,(sockaddr*)&server_addr,sizeof(sockaddr_in)))
        throw std::runtime_error("bind error code: " + std::to_string(WSAGetLastError()));
    if(listen(server_socket,SOMAXCONN))
        throw std::runtime_error("socket listening error code: " + std::to_string(WSAGetLastError()));
}

std::shared_ptr<HTTPrequest> HTTPServer::readRequest(SOCKET s)
{
    auto req = std::make_shared<HTTPrequest>();
    std::string header,body,lasted = "";
    do{ 
        unsigned char buff[1];
        recv(s,(char*)&buff,1,0);
        header += buff[0];
        if(header.length() >= 4)
            lasted = header.substr(header.length()-4,4);
    }while(lasted != "\r\n\r\n");
    *req = header;
    if(!req->Header().Params().count("Content-Length"))
        return req;
    int64_t data_size = std::stoull(req->Header().Params()["Content-Length"]);
    char *buff = new char[rwblock_size];
    while(data_size > 0){
       int readed = recv(s,buff,rwblock_size,0);
       if(readed == SOCKET_ERROR)
            throw std::runtime_error("socket error: error at reading");
       data_size -= readed;
       req->Data().append(buff,readed);
    }
    return req;
}

void HTTPServer::sendResponse(SOCKET s, const HTTPresponse &response)
{
    auto data_str_ptr = response.ToStr();
    int64_t size = data_str_ptr->size(); 
    const char* send_data =  data_str_ptr->c_str();
    uint32_t pos = 0;
    while(pos < size){
        int sended =  send(s,(const char*)&send_data[pos],(pos + rwblock_size > size)?(size - pos):rwblock_size,0);
        if(sended == SOCKET_ERROR)
            throw std::runtime_error("socket error: error at sending");
        pos += sended;
    }
}
void HTTPServer::work(SOCKET s,std::shared_ptr<HTTPrequest> req,in_addr ip)
{
    WaitForSingleObject(this->getMutexConsole(),INFINITE);
    std::cout << "Request ip: " << inet_ntoa(ip) << " Method: " << HTTPrequest::HTTPheader::methodToStr(req->Header().Method())
     << " URL: " << *req->Header().Url().toStr() << std::endl;
    ReleaseMutex(this->getMutexConsole());
    std::wstringstream wlog;
    wlog << L"Request: " << fromUTF8(inet_ntoa(ip)) << L" Method: "
     << fromUTF8(HTTPrequest::HTTPheader::methodToStr(req->Header().Method())) 
     << " URL: " << fromUTF8(*req->Header().Url().toStr());
    this->WriteTolog(wlog.str());
    wlog.clear();
    HTTPresponse response;
    std::wstring local_path = urldecode(req->Header().Url().PathStr().operator*());
    std::wstring abs_path = current_proj_path + local_path;
    std::string postfix = FilePostfix(local_path);
    if(!(req->Header().Version() == "1.1" || req->Header().Version() == "1.0")){
        response = *makeErrorResponse(505,"HTTP Version Not Supported");
        goto end;
    }
    if(!settings.isMethodAccess(local_path,req->Header().Method(),inet_ntoa(ip))){
        response = *makeErrorResponse(403,"Forbidden");
        goto end;
    }
    if(postfix.empty() && settings.getDefaultPostfix() == ".dll"){
        abs_path += fromUTF8(settings.getDefaultPostfix());
        response = *LoadDll(abs_path,req);
        goto end;
    }else if(postfix.empty() && settings.getDefaultPostfix() != ".dll"){
        if(!settings.getAssociation().count(postfix)){
            response = *makeErrorResponse(404,"Not Found");
            goto end;
        }
        response = *ExternalProcess(settings.getAssociation().find(postfix)->second,abs_path,req);
        goto end;
    }else if(postfix == ".dll"){
        response = *LoadDll(abs_path,req);
        goto end;
    }else if(settings.getAssociation().count(postfix)){
        response = *ExternalProcess(settings.getAssociation().find(postfix)->second,abs_path,req);
        goto end;
    }

    switch (req->Header().Method())
    {
    case METHOD::HEAD: response = *GET(req,true);break;
    case METHOD::GET: response = *GET(req);break;
    case METHOD::PUT: response = *PUT(req);break;
    case METHOD::DEL: response = *DEL(req);break;
    case METHOD::POST: response = *POST(req);break;
    default:
        break;
    }
    end:
    WaitForSingleObject(this->getMutexConsole(),INFINITE);
    std::cout << "Response ip: " << inet_ntoa(ip) << " "
     << response.Header().Status().first << response.Header().Status().second <<std::endl;
    ReleaseMutex(this->getMutexConsole());
    wlog << L"Response: " << fromUTF8(inet_ntoa(ip)) << L" " << std::to_wstring(response.Header().Status().first)
     << L" " << fromUTF8(response.Header().Status().second);
    this->WriteTolog(wlog.str());
    HttpDate current_date_time = HttpDate::getCurGmtTime();
    response.Header().Params().insert(std::make_pair("Date",current_date_time.toStr()));
    sendResponse(s,response);
}

HANDLE HTTPServer::getMutexConsole()
{
    return mutex_console;
}

HANDLE HTTPServer::getMutexLog()
{
    return mutex_log;
}

void HTTPServer::WriteTolog(const std::wstring str)
{
    WaitForSingleObject(this->getMutexLog(),INFINITE);
    std::string data = toUTF8(str) + " " + HttpDate::getCurGmtTime().toStr();
    std::ofstream target_file(path_to_log_file.c_str(),std::ios::binary | std::ios::app);
    if(!target_file.is_open())
        throw std::runtime_error("file was not open");
    target_file.write(data.c_str(),data.size());
    target_file.close();
    ReleaseMutex(this->getMutexLog());
}

const ServerSettings &HTTPServer::getSettings() const
{
    return this->settings;
}

void HTTPServer::setSettings(ServerSettings &settings)
{
    this->settings = settings;
}

std::shared_ptr<HTTPresponse> HTTPServer::GET(std::shared_ptr<HTTPrequest> req,bool isHead)const
{
    std::wstring abs_path = current_proj_path;
    if(req->Header().Url().PathStr()->empty())
        abs_path += L"\\" + settings.getDefaultResorce();
    else
        abs_path += urldecode(req->Header().Url().PathStr().operator*());
    std::string postfix = FilePostfix(abs_path);
    auto response = std::make_shared<HTTPresponse>();
    if(!isFileExist(abs_path)){
        return makeErrorResponse(404,"Not Found",isHead);
    }
    std::string content_type = "application/octet-stream";
    if(settings.getMimeTypes().count(postfix))
        content_type = settings.getMimeTypes().find(postfix).operator*().second;
    if(req->Header().Params().count("If-Modified-Since")){
        HttpDate dt_file = HttpDate::getFileGmtTime(abs_path);
        HttpDate dt_req = req->Header().Params()["If-Modified-Since"];
        if(dt_file <= dt_req)
            return makeErrorResponse(304,"Not Modified",isHead);
    }
    if(req->Header().Params().count("If-Unmodified-Since")){
        HttpDate dt_file = HttpDate::getFileGmtTime(abs_path);
        HttpDate dt_req = req->Header().Params()["If-Unmodified-Since"];
        if(dt_file > dt_req)
            return makeErrorResponse(412,"Precondition Failed",isHead);
    }
    if(req->Header().Params().count("Range")){
        auto range_data = std::make_unique<std::vector<std::pair<std::string,std::string>>>();
        auto ranges = split(req->Header().Params()["Range"],{",","="});
        size_t file_size = getFileSize(abs_path);
        std::string str_file_size = std::to_string(file_size);
        size_t k = 1;
        if(!file_size){
            return makeErrorResponse(416,"Range Not Satisfiable",isHead);
        }
        response->Header().Params().insert(std::make_pair("Accept-Ranges","bytes"));
        ranges->erase(ranges->begin());
        for(const auto& el: *ranges){
            std::string range_str = trim(el);
            if(range_str[0] == '-'){
                range_str.erase(range_str.begin());
                if(std::stoul(range_str) >= file_size)
                    return makeErrorResponse(416,"Range Not Satisfiable",isHead);
                range_str = "0-" + std::to_string(file_size - std::stoul(range_str));
            }
            if(range_str.back() == '-'){
                range_str.pop_back();
                if(std::stoul(range_str) >= file_size)
                    return makeErrorResponse(416,"Range Not Satisfiable",isHead);
                range_str = range_str + "-" + str_file_size;
            }
            auto beg_end = split(range_str,{"-"});
            size_t beg = 0, end = 0;
            if(beg_end->size() > 0)
                beg = std::stoul(trim(beg_end->at(0)));
            if(beg_end->size() > 1)
                end = std::stoul(trim(beg_end->at(1)));
            if(beg >= file_size || end > file_size || beg >= end)
                return makeErrorResponse(416,"Range Not Satisfiable",isHead);
            range_data->push_back(std::make_pair(*this->FileReadAll(abs_path,beg,end),range_str));
        }
        if(ranges->size() > 1){
            std::string boundary = this->boundary(*range_data);
            response->Header().Status() = std::make_pair<>(206,"Partial Content");
            response->Header().Params().insert(std::make_pair("Content-Type","multipart/byteranges; boundary="));
            response->Header().Params()["Content-Type"] += boundary;
            for(const auto& el: *range_data){
                response->Data() += boundary + "\r\n";
                response->Data() += "Content-Type: " + content_type + "\r\n";
                response->Data() += "Content-Range: " + el.second + "/" + str_file_size + "\r\n\r\n";
                response->Data().append(el.first);
                response->Data() += "\r\n\r\n";
            }
            response->Data().erase(response->Data().end() - 4,response->Data().end());
        }else{
            response->Header().Params().insert(std::make_pair("Content-Type",content_type));
            response->Header().Params().insert(std::make_pair("Content-Range",range_data->at(0).second += "/" + str_file_size));
            response->Data().append(range_data->at(0).first);
        }
    }else{
            response->Header().Params().insert(std::make_pair("Content-Type",content_type));
            response->Data() += *this->FileReadAll(abs_path);
    }
    std::string content_len = std::to_string(response->Data().size());
    response->Header().Params().insert(std::make_pair("Content-Length",content_len));
    if(isHead)
        response->Data().clear();
    return response;
}

std::shared_ptr<HTTPresponse> HTTPServer::PUT(std::shared_ptr<HTTPrequest> req)
{
    auto response = std::make_shared<HTTPresponse>();
    std::wstring abs_path = current_proj_path + urldecode(req->Header().Url().PathStr().operator*());
    if(!isFileExist(abs_path))
        response->Header().Status() = std::make_pair(201,"Created");
    FileWriteAll(abs_path,req->Data());
    response->Header().Params().insert(std::make_pair("Content-Location",req->Header().Url().PathStr().operator*()));
    return response;
}

std::shared_ptr<HTTPresponse> HTTPServer::DEL(std::shared_ptr<HTTPrequest> req)
{
    auto response = std::shared_ptr<HTTPresponse>();
    std::wstring abs_path = current_proj_path + urldecode(req->Header().Url().PathStr().operator*());
    if(!isFileExist(abs_path))
        return makeErrorResponse(404,"Not Found");
    if(!_wremove(abs_path.c_str()))
        throw std::runtime_error("file was not deleted");
    response->Header().Status() = std::make_pair(204,"No Content");
    return response;
}

std::shared_ptr<HTTPresponse> HTTPServer::POST(std::shared_ptr<HTTPrequest> req)
{
    auto response = std::make_shared<HTTPresponse>();
    std::wstring abs_path = current_proj_path + urldecode(req->Header().Url().PathStr().operator*());
    if((req->Header().Params().count("Content-Type"))
    && (req->Header().Params()["Content-Type"] == "application/x-www-form-urlencoded")
    || split(req->Header().Params()["Content-Type"],{"/"},1)->front() == "multipart"){
        return makeErrorResponse(400,"Bad Request");
    }
    if(!isFileExist(abs_path))
        response->Header().Status() = std::make_pair(201,"Created");
    response->Header().Params().insert(std::make_pair("Content-Location",req->Header().Url().PathStr().operator*()));
    FileAppend(abs_path,req->Data());
    return response;
}

std::shared_ptr<HTTPresponse> HTTPServer::LoadDll(std::wstring path,std::shared_ptr<HTTPrequest> req)
{
    if(!isFileExist(path) && settings.getDefaultHandlerFile().empty())
        return makeErrorResponse(404,"Not Found");
    else if(!isFileExist(path))
        path = this->current_proj_path + L"\\" + settings.getDefaultHandlerFile();
    HMODULE dynamic_lib = LoadLibraryW(path.c_str());
    HTTPresponse (*external_logic)(const HTTPrequest& req) = nullptr;
    external_logic = (HTTPresponse (*)(const HTTPrequest& req))GetProcAddress(dynamic_lib,"start"); 
    CloseHandle(dynamic_lib);
    if(!external_logic){
        return makeErrorResponse(500,"Internal Server Error");
    }
    return std::make_shared<HTTPresponse>(external_logic(*req));
}

size_t HTTPServer::getFileSize(const std::wstring &w_path)const{
    std::ifstream target_file(w_path.c_str(),std::ios::binary);
    if(!target_file.is_open())
        return 0;
    target_file.seekg(0,std::ios::end);
    return target_file.tellg();
}

std::shared_ptr<HTTPresponse> HTTPServer::ExternalProcess(std::wstring path_to_exe,const std::wstring path_file,std::shared_ptr<HTTPrequest> req)
{
    if(!isFileExist(path_file) && settings.getDefaultHandlerFile().empty())
        return makeErrorResponse(404,"Not Found");
    else if(!isFileExist(path_file))
        path_to_exe = this->current_proj_path + L"\\" + settings.getDefaultHandlerFile();
    HANDLE child_out_rd;
    HANDLE child_out_wr;
    SECURITY_ATTRIBUTES sattr;
    sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sattr.lpSecurityDescriptor = NULL;
    sattr.bInheritHandle = TRUE;
    if(!CreatePipe(&child_out_rd,&child_out_wr,&sattr,0))
        throw std::runtime_error("pipe not created");
    std::fstream is;
    STARTUPINFOW si;
    ZeroMemory(&si,sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = child_out_wr;
    si.hStdOutput = child_out_wr;
    si.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION pi{0};
    if(!CreateProcessW(path_to_exe.c_str(),(wchar_t*)(std::wstring(L" ") + path_file + L" "  + fromUTF8(*req->toStr())).c_str(),NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)){
        throw std::runtime_error("proccess not created");
    }
    WaitForSingleObject(pi.hProcess,1000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    std::string message = "";
    unsigned long avail = 0;
    do{
        char *buff = new char[avail];
        if(avail){
            if(!ReadFile(child_out_rd,buff,avail,NULL,0)){
                CloseHandle(child_out_rd);
                CloseHandle(child_out_wr);
                throw std::runtime_error("pipe is broken");
            }
            message.append(buff,avail);
        }
        if(!PeekNamedPipe(child_out_rd,NULL,0,NULL,&avail,NULL)){
            CloseHandle(child_out_rd);
            CloseHandle(child_out_wr);
            throw std::runtime_error("pipe is broken");
        }
        delete buff;
    }while (avail);
    CloseHandle(child_out_rd);
    CloseHandle(child_out_wr);
    return std::make_shared<HTTPresponse>(message);
}

std::shared_ptr<std::string> HTTPServer::FileReadAll(const std::wstring &w_path, size_t off, size_t len) const
{ 
    std::ifstream target_file(w_path.c_str(),std::ios::binary);
    if(!target_file.is_open())
        throw std::runtime_error("file was not open");
    auto result = std::make_shared<std::string>();
    target_file.seekg(off,std::ios::beg);
    char buff;
    uint64_t readed_length = 0;
    while(!target_file.eof() && (readed_length < len || len <= 0)){
        target_file.get(buff);
        *result += buff;
        ++readed_length;
    }
    if(len < 0)
        *result = result->substr(0,result->size()+len);
    target_file.close();
    return result;
}

void HTTPServer::FileAppend(const std::wstring &path,const std::string &data)
{
    std::ofstream target_file(path.c_str(),std::ios::binary | std::ios::app);
    if(!target_file.is_open())
        throw std::runtime_error("file was not open");
    target_file.write(data.c_str(),data.size());
    target_file.close();
}

void HTTPServer::FileWriteAll(const std::wstring &path,const std::string &data)
{
    std::ofstream target_file(path.c_str(),std::ios::binary);
    if(!target_file.is_open())
        throw std::runtime_error("file was not open");
    target_file.write(data.c_str(),data.size());
    target_file.close();
}

std::string HTTPServer::FilePostfix(const std::wstring &path) const
{
    size_t found = path.find_last_of('.');
    if(found == std::string::npos)
        return "";
    std::wstring postfix = path.substr(found,path.length()-1);
    std::string res(postfix.begin(),postfix.end());
    return res;
}

std::string HTTPServer::boundary(const std::vector<std::pair<std::string,std::string>> & data) const
{
    std::string boundary = "";
    auto full_data = std::make_unique<std::string>();
    uint64_t size = 0;
    for(const auto& elem: data){
        size += elem.first.size();
        full_data->append(elem.first);
    }
    if(!size)
        return "";
    uint64_t boundary_size = 4 + std::log2(size) / 2;
    std::mt19937_64 rand_eng;
    bool is_unique = true;
    do{
        boundary.clear();
        boundary = "----";
        rand_eng.seed(time(NULL));
        for(uint64_t i = 0;i < boundary_size;++i)
            boundary.append(std::to_string(rand_eng() % 10));
        boundary.append("----");
        for(uint64_t i = 0; i < size; ++i){
            if(full_data->substr(i,boundary_size) == boundary){
                is_unique = false;
                break;
            }
        }
    }while(!is_unique);
    return boundary;
}

std::shared_ptr<HTTPresponse> HTTPServer::makeErrorResponse(uint32_t status_code,const std::string& status,bool isHead) const
{
    auto res = std::make_shared<HTTPresponse>();
    res->Header().Status() = std::make_pair(status_code,status);
    HttpDate current_date_time = HttpDate::getCurGmtTime();
    res->Header().Params().insert(std::make_pair("Date",current_date_time.toStr()));
    std::wstring path = std::to_wstring(status_code) + L"html";
    if(isFileExist(path) && !isHead){
        res->Data().append(*FileReadAll(path));
    }
    return res;
}

bool HTTPServer::isFileExist(const std::wstring &path)const
{
    DWORD res = GetFileAttributes(path.c_str());
    return (res != INVALID_FILE_ATTRIBUTES) && !(res & FILE_ATTRIBUTE_DIRECTORY);
}

void HTTPServer::exec(){
    std::cout << "Server started\n";
    SOCKET new_socket = INVALID_SOCKET;
    sockaddr_in new_addr{0};
    int addr_size = sizeof(new_addr);
    while(true){
        std::cout << "Listen...\n";
        new_socket = accept(server_socket,(sockaddr*)&new_addr,&addr_size);
        if(new_socket == INVALID_SOCKET)
            continue;
        std::cout << "Connect " << "ip: " << inet_ntoa(new_addr.sin_addr)
        << " Socket: " << std::to_string(new_socket) 
        << " Date:" << HttpDate::getCurGmtTime().toStr() << std::endl;
        std::wstring post_log = L"Connect " + fromUTF8(inet_ntoa(new_addr.sin_addr)) + L"\n";
        this->WriteTolog(post_log);
        auto *params = new std::tuple<IHTTPServer*,SOCKET,in_addr>(this,new_socket,new_addr.sin_addr);
        PTP_WORK new_work = CreateThreadpoolWork(clientThread,params,th_pool_env.get());
        if(!new_work){
            shutdown(new_socket,SD_BOTH);
            closesocket(new_socket);
            continue;
        }
        SubmitThreadpoolWork(new_work);
    }
}