#include "../lib/OtherFunction.h"

std::shared_ptr<std::vector<std::string>> split(
    const std::string &str,
    const std::initializer_list<std::string>& list,
    size_t max_iter)
{
    std::shared_ptr<std::vector<std::string>> result = std::make_shared<std::vector<std::string>>();
    size_t (*find)(const std::string& str,std::string sep,size_t) = [](const std::string& str,std::string sep,size_t p){
        return str.find(sep,p);
    };
    if(list.size()>1){
        find = [](const std::string& str,std::string sep,size_t p){
            return str.find_first_of(sep,p);
        };
    }
    std::string sep = "";
    if(!list.size())
        sep += " ";
    for(const auto& el : list)
        sep+= el;
    size_t pos = 0;
    size_t last_pos = 0;
    size_t step = 0;
    while((pos=find(str,sep,last_pos)) != std::string::npos && step < max_iter){
        std::string sub_str = str.substr(last_pos,pos-last_pos);
        if(!sub_str.empty())
            result->push_back(sub_str);
        last_pos = pos + ((list.size()>1) ? 1 : sep.length());
        ++step;
    }
    if(last_pos < str.length())
        result->push_back(str.substr(last_pos,str.length() - last_pos));
    return result;
}


std::wstring fromUTF8(const std::string& src){
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(src);
}

std::string toUTF8(const std::wstring & src){
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(src);
}

std::wstring urldecode(const std::string& src){
    std::string res;
    std::string s_num = "";
    for(size_t i = 0;i < src.size();++i){
        if(src[i] == '%'){
            s_num = src.substr(++i,2);
            int num = std::stoi("0x" + s_num,0,16);
            s_num = static_cast<unsigned char>(num);
            res += s_num;
            ++i;
        }else{
            res += src[i];
        }
    }
    if(res.empty())
        res = src;
    return fromUTF8(res);
}

std::string urlncode(const std::wstring& src){
    std::string utf8_str = toUTF8(src), result;
    char buff[2];
    for(const auto& el: utf8_str){
        itoa(static_cast<uint8_t>(el),buff,16);
        result.append("%");
        result += std::toupper(buff[0]);
        result += std::toupper(buff[1]);
    }
    return result;
}

std::string& trim(std::string& str){
    for(size_t i = 0; i < str.size();++i){
        if(!isspace(str[i]))
            break;
        str.erase(i,1);
    }
    for(std::string::reverse_iterator ri = str.rbegin(); ri != str.rend();++ri){
        if(!isspace(*ri))
            break;
        str.pop_back();
    }
    return str;
}

std::string trim(const std::string& str){
    std::string res = str;
    for(size_t i = 0; i < str.size();++i){
        if(!isspace(str[i]))
            break;
        res.erase(i,1);
    }
    for(auto ri = str.rbegin(); ri != str.rend();++ri){
        if(!isspace(*ri))
            break;
        res.pop_back();
    }
    return res;
}

std::string replaceAll(std::string src,const std::string str,const std::string rep_str){
    size_t pos = 0;
    while((pos = src.find(str,pos)) != std::string::npos){
        src = src.replace(pos,1,rep_str);
    }
    return src;
}
