#ifndef OTHERH
#define OTHERH
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <codecvt>
#include <locale>

std::shared_ptr<std::vector<std::string>> split(
    const std::string &str,
    const std::initializer_list<std::string>& list,
    size_t max_iter = std::string::npos);

std::string& trim(std::string& str);
std::string trim(const std::string& str);

std::wstring urldecode(const std::string& src);
std::string urlncode(const std::wstring& src);

std::wstring fromUTF8(const std::string& src);
std::string toUTF8(const std::wstring & src);

std::string replaceAll(std::string src,const std::string str,const std::string rep_str);
#endif