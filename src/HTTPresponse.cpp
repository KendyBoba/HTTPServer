#include "../lib/HTTPresponse.h"

HTTPresponse::HTTPheader::HTTPheader():
    params(std::make_shared<std::map<std::string,std::string>>()),
    status(std::make_pair(200,"OK")),
    cookie(std::make_shared<std::map<std::string,std::string>>())
{

}

HTTPresponse::HTTPheader::HTTPheader(const std::string & src): 
    HTTPresponse::HTTPheader::HTTPheader()
{
    Parse(src);
}

HTTPresponse::HTTPheader::HTTPheader(const HTTPresponse::HTTPheader & src):
    HTTPresponse::HTTPheader::HTTPheader()
{
    *params = *src.params;
    *cookie = *src.cookie;
}

HTTPresponse::HTTPheader::HTTPheader(HTTPresponse::HTTPheader && src){
    params = std::move(src.params);
    cookie = std::move(src.cookie);
    ver = src.ver;
    status = std::move(src.status);
}

HTTPresponse::HTTPheader& HTTPresponse::HTTPheader::operator=(const std::string & src){
    if(!params)
        params = std::make_shared<std::map<std::string,std::string>>();
    if(!cookie)
        cookie = std::make_shared<std::map<std::string,std::string>>();
    Parse(src);
    return *this;
}

HTTPresponse::HTTPheader& HTTPresponse::HTTPheader::operator=(const HTTPresponse::HTTPheader & src)
{
    if(!params)
        params = std::make_shared<std::map<std::string,std::string>>();
    if(!cookie)
        cookie = std::make_shared<std::map<std::string,std::string>>();
    *cookie = *src.cookie;
    *params = *src.params;
    ver = src.ver;
    status = src.status;
    return *this;
}

HTTPresponse::HTTPheader& HTTPresponse::HTTPheader::operator=(HTTPresponse::HTTPheader && src){
    cookie = std::move(src.cookie);
    params = std::move(src.params);
    ver = src.ver;
    status = std::move(src.status);
    return *this;
}

std::pair<unsigned short,std::string>& HTTPresponse::HTTPheader::Status(){
    return status;
}

std::string& HTTPresponse::HTTPheader::Version(){
    return ver;
}

std::map<std::string,std::string>& HTTPresponse::HTTPheader::Params(){
    return *params;
}

std::map<std::string,std::string>& HTTPresponse::HTTPheader::Cookie(){
    return *cookie;
}

void HTTPresponse::HTTPheader::CookieParse(const std::string& src){
    if(src.empty())
        return;
    auto cookie_list = split(src,{";"});
    for(const auto& el : *cookie_list){
        auto key_val = split(el,{"="});
        if(key_val->empty())
            continue;
        this->cookie->insert(std::make_pair(trim(key_val->at(0)),(key_val->size() > 1)?trim(key_val->at(1)):""));
    }
}

std::string HTTPresponse::HTTPheader::CookieToStr()const{
    std::string result = "";
    for(const auto &el: *cookie){
        result.append(el.first);
        if(!el.second.empty()){
            result.append("=" + el.second);
        }
        result.append(";");
    }
    return result;
}

void HTTPresponse::HTTPheader::Parse(const std::string & src){
    this->Clear();
    if(src.empty())
        return;
    auto lines = split(src,{"\r\n"});
    if(lines->empty())
        return;
    auto first_line = split(lines->at(0),{" "},2);
    if(first_line->size() < 3)
        throw std::invalid_argument("header is invalid");
    ver = split(first_line->at(0),{"\\","/"},1)->at(1);
    status.first = std::stoi(first_line->at(1));
    status.second = first_line->at(2);
    for(unsigned long long i = 1; i < lines->size();++i){
        auto temp = split(lines->at(i),{":"},1);
        if(temp->at(0) == "Set-Cookie"){
            CookieParse(temp->at(1));
            continue;
        }
        if(temp->size() >=2)
            params->insert(std::make_pair(trim(temp->at(0)),trim(temp->at(1))));
    }
}

void HTTPresponse::HTTPheader::Clear(){
    cookie->clear();
    params->clear();
    status.first = 200;
    status.second = "OK";
    ver = "1.1";
}

std::shared_ptr<std::string> HTTPresponse::HTTPheader::toStr(){
    auto result = std::make_shared<std::string>();
    result->append("HTTP/" + Version() + " ");
    result->append(std::to_string(Status().first) + " " + Status().second);
    result->append("\r\n");
    for(const auto& el : Params())
        result->append(el.first + ": " + el.second + "\r\n");
    if(!cookie->empty()){
        result->append("Set-Cookie: ");
        result->append(CookieToStr());
        result->append("\r\n");
    }
    result->append("\r\n\r\n");
    return result;
}

HTTPresponse::HTTPresponse():
header(std::make_shared<HTTPresponse::HTTPheader>()),
data(std::make_shared<std::string>())
{

}

HTTPresponse::HTTPresponse(const std::string& src):
header(std::make_shared<HTTPresponse::HTTPheader>()),
data(std::make_shared<std::string>())
{
    Parse(src);
}

HTTPresponse::HTTPresponse(const HTTPresponse& src):
header(std::make_shared<HTTPresponse::HTTPheader>()),
data(std::make_shared<std::string>())
{
    *header = *src.header;
    *data = *src.data;
}

HTTPresponse::HTTPresponse(HTTPresponse&& src){
    header = std::move(src.header);
    data = std::move(src.data);
}

HTTPresponse &HTTPresponse::operator=(HTTPresponse &&src)
{
    header = std::move(src.header);
    data = std::move(src.data);
    return *this;
}

HTTPresponse & HTTPresponse::operator=(const HTTPresponse & src)
{
    if(!header)
        header = std::make_shared<HTTPresponse::HTTPheader>();
    if(!data)
        data = std::make_shared<std::string>();
    *header = *src.header;
    *data = *src.data;
    return *this;
}

HTTPresponse & HTTPresponse::operator=(const std::string & src)
{
    if(!header)
        header = std::make_shared<HTTPresponse::HTTPheader>();
    if(!data)
        data = std::make_shared<std::string>();
    Parse(src);
    return *this;
}

void HTTPresponse::Parse(const std::string& src){
    if(src.empty())
        return;
    this->Clear();
    auto header_data = split(src,{"\r\n\r\n"},1);
    *header = header_data->at(0);
    if(header_data->size() <= 1)
        return;
    *data = header_data->at(1);
}

HTTPresponse::HTTPheader& HTTPresponse::Header(){
    return *this->header;
}

std::string &HTTPresponse::Data()
{
    return *this->data;
}

std::shared_ptr<std::string> HTTPresponse::ToStr()const
{
    auto result = std::make_shared<std::string>();
    result->append(*header->toStr());
    result->append(*this->data);
    return result;
}

void HTTPresponse::Clear()
{
    header->Clear();
    data->clear();
}
