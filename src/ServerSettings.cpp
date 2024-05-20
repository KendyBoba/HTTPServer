#include "../headers/ServerSettings.h"

ServerSettings::ServerSettings():
    mime_types(std::make_unique<std::unordered_map<std::string,std::string>>()),
    access_resource(std::make_unique<std::vector<access_info>>()),
    association_parser(std::make_unique<std::map<std::string,std::wstring>>())
{

}

ServerSettings::ServerSettings(const std::wstring &mime_path, const std::wstring &access_path, const std::wstring &settings_path)
    :ServerSettings()
{
    ReadMimeTypes(mime_path);
    ReadAccesMethod(access_path);
    ReadSettings(settings_path);
}

ServerSettings::ServerSettings(const ServerSettings &settings) : ServerSettings()
{
    *this->access_resource = *settings.access_resource;
    *this->mime_types = *settings.mime_types;
    *this->association_parser = *settings.association_parser;
    this->main_dir = settings.main_dir;
    this->max_conn = settings.max_conn;
    this->max_threads = settings.max_threads;
    this->max_time_wait_sec = settings.max_time_wait_sec;
    this->port = settings.port;
    this->ip = settings.ip;
    this->default_file = settings.default_file;
}

ServerSettings::ServerSettings(ServerSettings &&settings)
{
    this->access_resource = move(settings.access_resource);
    this->mime_types = move(settings.mime_types);
    this->association_parser = move(settings.association_parser);
    this->main_dir = move(settings.main_dir);
    this->max_conn = settings.max_conn;
    this->max_threads = settings.max_threads;
    this->max_time_wait_sec = settings.max_time_wait_sec;
    this->port = settings.port;
    this->ip = settings.ip;
    this->default_file = move(settings.default_file);
}

ServerSettings &ServerSettings::operator=(const ServerSettings &settings)
{
    *this->access_resource = *settings.access_resource;
    *this->mime_types = *settings.mime_types;
    *this->association_parser = *settings.association_parser;
    this->main_dir = settings.main_dir;
    this->max_conn = settings.max_conn;
    this->max_threads = settings.max_threads;
    this->max_time_wait_sec = settings.max_time_wait_sec;
    this->port = settings.port;
    this->ip = settings.ip;
    this->default_file = settings.default_file;
    return *this;
}

ServerSettings &ServerSettings::operator=(ServerSettings &&settings)
{
    this->access_resource = move(settings.access_resource);
    this->mime_types = move(settings.mime_types);
    this->association_parser = move(settings.association_parser);
    this->main_dir = move(settings.main_dir);
    this->max_conn = settings.max_conn;
    this->max_threads = settings.max_threads;
    this->max_time_wait_sec = settings.max_time_wait_sec;
    this->port = settings.port;
    this->ip = settings.ip;
    this->default_file = move(settings.default_file);
    return *this;
}

void ServerSettings::ReadMimeTypes(const std::wstring &path)
{
    std::ifstream file(path.c_str());
    if(!file.is_open())
        throw std::runtime_error("file was not open");
    std::string line;
    while(std::getline(file,line)){
        auto pair = split(line,{},1);
        mime_types->insert(std::make_pair(trim(pair->at(0)),trim(pair->at(1))));
    }
    file.close();
}

void ServerSettings::ReadAccesMethod(const std::wstring &path)
{
    std::ifstream file(path.c_str());
    if(!file.is_open())
        throw std::runtime_error("file was not open");
    access_info aci;
    std::string line;
    while(std::getline(file,line)){
        auto params = split(line,{},3);
        aci.path = fromUTF8(trim(params->at(0)));
        aci.ip = trim(params->at(2));
        aci.method = trim(params->at(1));
        aci.isAllow = (trim(params->at(3)) == "Allow")? true:false;
        access_resource->push_back(aci);
    }
    file.close();
}

void ServerSettings::ReadSettings(const std::wstring &path)
{
    std::ifstream file(path.c_str());
    if(!file.is_open())
        throw std::runtime_error("file was not open");
    std::string line;
    while(std::getline(file,line)){
        auto params = split(line,{"="},1);
        std::string key = trim(params->at(0)),val = trim(params->at(1));
        if(key[0] == '#')
            continue;
        else if(key == "main_dir")
            this->main_dir = fromUTF8(val);
        else if(key == "max_threads")
            this->max_threads = std::stoi(val);
        else if(key == "max_conn")
            this->max_conn = std::stoi(val);
        else if(key == "max_time_wait_sec")
            this->max_time_wait_sec = std::stoi(val);
        else if(key == "port")
            this->port = std::stoi(val);
        else if(key == "default_file")
            this->default_file = fromUTF8(val);
        else if(key == "server_ip")
            this->ip = inet_addr(val.c_str());
        else if(key[0] == '.'){
            std::wstring path = fromUTF8(val);
            association_parser->insert(std::make_pair(key,path));
        }else if(key == "default_postfix"){
            this->default_postfix = val;
        }else if(key == "default_handler")
            this->default_handler = fromUTF8(val);
    }
    file.close();
}

const std::unordered_map<std::string, std::string> &ServerSettings::getMimeTypes()const
{
    return *this->mime_types;
}

const std::map<std::string, std::wstring> &ServerSettings::getAssociation() const
{
    return *this->association_parser;
}

bool ServerSettings::isMethodAccess(const std::wstring &path,METHOD method,const std::string& ip) const
{
    auto found = std::find_if(access_resource->begin(),access_resource->end(),[&](const access_info & el)->bool{
            return (compare(path,el.path) && compare(ip,el.ip) && compare(HTTPrequest::HTTPheader::methodToStr(method),el.method))?true:false;
    });
    if(found == access_resource->end())
        return false;
    return (*found).isAllow;
}

const std::wstring &ServerSettings::getMainDir() const
{
    return this->main_dir;
}

const std::wstring &ServerSettings::getDefaultResorce() const
{
    return this->default_file;
}

const std::string &ServerSettings::getDefaultPostfix() const
{
    return this->default_postfix;
}

const std::wstring &ServerSettings::getDefaultHandlerFile() const
{
    return this->default_handler;
}

uint16_t ServerSettings::getMaxThreads() const
{
    return this->max_threads;
}

uint16_t ServerSettings::getMaxConnections() const
{
    return this->max_conn;
}

uint16_t ServerSettings::getTimeWait() const
{
    return this->max_time_wait_sec;
}

uint16_t ServerSettings::getPort() const
{
    return this->port;
}

unsigned long ServerSettings::getIp() const
{
    return this->ip;
}

bool ServerSettings::compare(const std::string &text, const std::string &mask) const
{
    std::wstring w_text(text.begin(),text.end());
    std::wstring w_mask(mask.begin(),mask.end());
    return compare(w_text,w_mask);
}

bool ServerSettings::compare(const std::wstring &text, const std::wstring &mask)const
{
    for(size_t i = 0,j = 0; j < text.size();++j){
         if(mask[i] == '*'){
            ++i;
            while (i < mask.size() && mask[i] == '?'){++i;}
            if(i >= mask.size())
                return true;
            size_t pos = text.find(mask[i],j);
            if(pos == std::string::npos)
                return false;
            j  = pos - 1;
        }else if(mask[i] == '?' && text[j] != '\\' && text[j] != '/'){
            ++i;
            continue;
        }else{
            if(mask[i] != text[j])
                return false;
            ++i;
        }
    }
    return true;
}
