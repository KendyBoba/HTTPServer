#ifndef RESPONSE
#define RESPONSE
#include <string>
#include <memory>
#include <map>
#include "URL.h"

class HTTPresponse{
public:
    class HTTPheader{
    private:
        std::pair<unsigned short,std::string> status; 
        std::string ver = "1.1";
        std::shared_ptr<std::map<std::string,std::string>> params;
        std::shared_ptr<std::map<std::string,std::string>> cookie;
    private:
        void Parse(const std::string & src);
        void CookieParse(const std::string& src);
        std::string CookieToStr()const;
    public:
        HTTPheader();
        HTTPheader(const std::string & src);
        HTTPheader(const HTTPheader & src);
        HTTPheader(HTTPheader && src);
        HTTPheader& operator=(const std::string & src);
        HTTPheader& operator=(const HTTPheader & src);
        HTTPheader& operator=(HTTPheader && src);
        std::shared_ptr<std::string> toStr();
        std::pair<unsigned short,std::string>& Status();
        std::string& Version();
        std::map<std::string,std::string>& Params();
        std::map<std::string,std::string>& Cookie();
        void Clear(); 
    };
private:
    std::shared_ptr<HTTPresponse::HTTPheader> header;
    std::shared_ptr<std::string> data;
private:
    void Parse(const std::string& src);
public:
    HTTPresponse();
    HTTPresponse(const std::string& src);
    HTTPresponse(const HTTPresponse& src);
    HTTPresponse(HTTPresponse&& src);
    HTTPresponse& operator=(const std::string& src);
    HTTPresponse& operator=(const HTTPresponse& src);
    HTTPresponse& operator=(HTTPresponse&& src);
    HTTPresponse::HTTPheader& Header();
    std::string& Data();
    std::shared_ptr<std::string> ToStr()const;
    void Clear();
};

#endif