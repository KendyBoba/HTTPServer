#include "../lib/HTTPrequest.h"

HTTPrequest::HTTPrequest():
header(std::make_shared<HTTPheader>()),
data(std::make_shared<std::string>())
{

}

HTTPrequest::HTTPrequest(const std::string &src): HTTPrequest::HTTPrequest()
{
    Parse(src);
}

HTTPrequest::HTTPrequest(const HTTPrequest &src): HTTPrequest::HTTPrequest()
{
    *header = *src.header;
    *data = *src.data;
}

HTTPrequest::HTTPrequest(HTTPrequest &&src)
{
    header = std::move(src.header);
    data = std::move(src.data);
}

HTTPrequest &HTTPrequest::operator=(const std::string &src)
{
    if(!header)
        header = std::make_shared<HTTPheader>();
    if(!data)
        data = std::make_shared<std::string>();
    Parse(src);
    return* this;
}

HTTPrequest &HTTPrequest::operator=(const HTTPrequest &src)
{
    if(!header)
        header = std::make_shared<HTTPheader>();
    if(!data)
        data = std::make_shared<std::string>();
    *header = *src.header;
    *data = *src.data;
    return *this;
}

HTTPrequest& HTTPrequest::operator=(HTTPrequest &&src)
{
    header = std::move(src.header);
    data = std::move(src.data);
    return *this;
}

void HTTPrequest::Parse(const std::string& src)
{
    if(src.empty())
        return;
    this->Clear();
    auto header_data = split(src,{"\r\n\r\n"},1);
    *header = header_data->at(0);
    if(header_data->size() <= 1)
        return;
    *data = header_data->at(1);
}

std::shared_ptr<std::string> HTTPrequest::toStr() const
{
    auto result = std::make_shared<std::string>();
    result->append(*(header->toStr()));
    result->append(*data);
    return result;
}

HTTPrequest::HTTPheader& HTTPrequest::Header()
{
    return *header;
}

std::string& HTTPrequest::Data()
{
    return *data;
}

void HTTPrequest::Clear()
{
    header->Clear();
    data->clear();
}

HTTPrequest::HTTPheader::HTTPheader():
    params(std::make_shared<std::map<std::string,std::string>>()),
    url(std::make_shared<URL>()),
    cookie(std::make_shared<std::map<std::string,std::string>>())
{
    
}

HTTPrequest::HTTPheader::HTTPheader(const std::string &header_str) : HTTPrequest::HTTPheader::HTTPheader()
{
    Parse(header_str);
}

METHOD HTTPrequest::HTTPheader::strToMethod(const std::string& str)
{
    if(str == "GET")
        return METHOD::GET;
    if(str == "POST")
        return METHOD::POST;
    if(str == "HEAD")
        return METHOD::HEAD;
    if(str == "PUT")
        return METHOD::PUT;
    if(str == "DELETE")
        return METHOD::DEL;
    throw std::domain_error("unexpected method");
}

std::string HTTPrequest::HTTPheader::methodToStr(METHOD method)
{
    switch (method)
    {
        case METHOD::GET: return "GET";break;
        case METHOD::POST: return "POST";break;
        case METHOD::PUT: return "PUT";break;
        case METHOD::HEAD: return "HEAD";break;
        case METHOD::DEL: return "DELETE";break;
        default: std::domain_error("unexpected method");break;
    }
}

HTTPrequest::HTTPheader::HTTPheader(const HTTPheader &header): HTTPrequest::HTTPheader::HTTPheader()
{
    *url = *header.url;
    *params = *header.params;
    method = header.method;
    *cookie = *header.cookie;
}

HTTPrequest::HTTPheader::HTTPheader(HTTPheader &&header)
{
    this->method = header.method;
    this->url = std::move(header.url);
    this->params = std::move(header.params);
    this->cookie = std::move(header.cookie);
    this->ver = header.ver; 
}

HTTPrequest::HTTPheader& HTTPrequest::HTTPheader::operator=(const std::string &header_str)
{
    if(!url)
        url = std::make_shared<URL>();
    if(!params)
        params = std::make_shared<std::map<std::string,std::string>>();
    if(!cookie)
        cookie = std::make_shared<std::map<std::string,std::string>>();
    Parse(header_str);
    return *this;
}

HTTPrequest::HTTPheader& HTTPrequest::HTTPheader::operator=(const HTTPheader &header)
{
    if(!url)
        url = std::make_shared<URL>();
    if(!params)
        params = std::make_shared<std::map<std::string,std::string>>();
    if(!cookie)
        cookie = std::make_shared<std::map<std::string,std::string>>();
    *cookie = *header.cookie;
    *url = *header.url;
    *params = *header.params;
    method = header.method;
    return *this;
}

HTTPrequest::HTTPheader& HTTPrequest::HTTPheader::operator=(HTTPheader &&header)
{
    this->method = header.method;
    this->url = std::move(header.url);
    this->params = std::move(header.params);
    this->cookie = std::move(header.cookie);
    this->ver = header.ver; 
    return *this;
}

METHOD &HTTPrequest::HTTPheader::Method()
{
    return method;
}

URL &HTTPrequest::HTTPheader::Url()
{
    return *url;
}

std::map<std::string, std::string> &HTTPrequest::HTTPheader::Params()
{
    return *params;
}

std::string &HTTPrequest::HTTPheader::Version()
{
    return ver;
}

std::string HTTPrequest::HTTPheader::CookieToStr()const{
    std::string result = "";
    for(const auto &el: *cookie){
        result.append(el.first);
        if(!el.second.empty()){
            result.append("=" + el.second);
        }
        result.append("; ");
    }
    return result;
}

std::shared_ptr<std::string> HTTPrequest::HTTPheader::toStr() const
{
    replaceAll(*this->url->toStr()," ","+");
    auto result = std::make_shared<std::string>();
    std::stringstream ss;
    ss << methodToStr(this->method) << " " << replaceAll(*this->url->toStr()," ","+") << " HTTP/" << ver << "\r\n";
    result->append(ss.str());
    for(const auto& el: *this->params){
        result->append(el.first);
        result->append(": ");
        result->append(el.second);
        result->append("\r\n");
    }
    if(!cookie->empty()){
        result->append("Cookie: ");
        result->append(CookieToStr());
        result->append("\r\n");
    }
    result->append("\r\n\r\n");
    return result;
}

void HTTPrequest::HTTPheader::Clear()
{
    params->clear();
    url->Clear();
    ver.clear();
    cookie->clear();
    method = METHOD::NIL;
}

void HTTPrequest::HTTPheader::CookieParse(const std::string& src){
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

void HTTPrequest::HTTPheader:: Parse(const std::string &src)
{
    if(src.empty())
        return;
    this->Clear();
    auto lines = split(src,{"\r\n"});
    auto first_line = split(lines->at(0),{" "},2);
    if(first_line->size() < 3)
        throw std::invalid_argument("header is invalid");
    method = this->strToMethod(first_line->at(0));
    *url = replaceAll(first_line->at(1),"+"," ");
    ver = split(first_line->at(2),{"\\","/"})->at(1);
    for(auto it = lines->begin()+1;it != lines->end();++it){
        auto key_val = split(*it,{":"},1);
        if(key_val->at(0) == "Cookie"){
            CookieParse(key_val->at(1));
            continue;
        }
        if(key_val->size() >= 2)
            params->insert(std::make_pair(trim(key_val->at(0)),trim(key_val->at(1))));
    }
    
}

std::map<std::string,std::string>& HTTPrequest::HTTPheader::Cookie(){
    return *cookie;
}
