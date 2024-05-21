#ifndef URLH
#define URLH
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <initializer_list>
#include "../lib/OtherFunction.h"

class URL{
private:
    std::shared_ptr<std::vector<std::string>> path;
    std::shared_ptr<std::map<std::string,std::string>> params;
    void Parse(const std::string &src);
public:
    URL();
    URL(const std::string &src);
    URL(const URL & obj);
    URL(URL&& obj);
    URL& operator=(const std::string& str);
    URL& operator=(const URL& obj);
    URL& operator=(URL&& obj);
    std::vector<std::string>& UrlList();
    std::shared_ptr<std::string> PathStr()const;
    std::map<std::string,std::string>& Params();
    std::shared_ptr<std::string> toStr()const;
    void Clear();
 };
 #endif