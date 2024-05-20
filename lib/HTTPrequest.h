#ifndef REQ
#define REQ
#include <map>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include "URL.h"

enum class METHOD{
    NIL,
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
};

class HTTPrequest{
public:
    class HTTPheader{
    private:
        std::shared_ptr<URL> url;
        METHOD method = METHOD::NIL;
        std::shared_ptr<std::map<std::string,std::string>> params;
        std::string ver = "";
        std::shared_ptr<std::map<std::string,std::string>> cookie;
    private:
        void Parse(const std::string& src); 
        void CookieParse(const std::string& src);
        std::string CookieToStr()const;
    public:
        HTTPheader();
        HTTPheader(const std::string& header_str);
        HTTPheader(const HTTPheader& header);
        HTTPheader(HTTPheader&& header);
        HTTPheader& operator=(const std::string& header_str);
        HTTPheader& operator=(const HTTPheader& header);
        HTTPheader& operator=(HTTPheader&& header);
    public:
        static METHOD strToMethod(const std::string& str);
        static std::string methodToStr(METHOD method);
        METHOD& Method();
        URL& Url();
        std::map<std::string,std::string>& Params();
        std::map<std::string,std::string>& Cookie();
        std::string& Version();
        std::shared_ptr<std::string> toStr()const;
        void Clear();
    };
public:
    HTTPrequest();
    HTTPrequest(const std::string& src);
    HTTPrequest(const HTTPrequest& src);
    HTTPrequest(HTTPrequest&& src);
    HTTPrequest& operator=(const std::string& src);
    HTTPrequest& operator=(const HTTPrequest& src);
    HTTPrequest& operator=(HTTPrequest&& src);
private:
    void Parse(const std::string& src);
public:
    std::shared_ptr<std::string> toStr()const;
    HTTPheader& Header();
    std::string& Data();
    void Clear();
private:
    std::shared_ptr<HTTPheader> header;
    std::shared_ptr<std::string> data;
};

#endif