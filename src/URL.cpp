#include "../lib/URL.h"

URL::URL(): 
path(std::make_shared<std::vector<std::string>>()),
params(std::make_shared<std::map<std::string,std::string>>())
{
}

URL::URL(const std::string& src):
path(std::make_shared<std::vector<std::string>>()),
params(std::make_shared<std::map<std::string,std::string>>())
{
    Parse(src);
}

URL::URL(const URL & obj):
path(std::make_shared<std::vector<std::string>>()),
params(std::make_shared<std::map<std::string,std::string>>())
{
    *path = *obj.path;
    *params = *obj.params;
}

URL& URL::operator=(const std::string& str){
    if(!path)
        path = std::make_shared<std::vector<std::string>>();
    if(!params)
        params = std::make_shared<std::map<std::string,std::string>>(); 
    Parse(str);
    return *this;
}

URL& URL::operator=(const URL& obj){
    if(&obj == this)
        return *this;
    if(!path)
        path = std::make_shared<std::vector<std::string>>();
    if(!params)
        params = std::make_shared<std::map<std::string,std::string>>(); 
    *path = *obj.path;
    *params = *obj.params;
     return *this;
}

URL::URL(URL&& obj){
    path = std::move(obj.path);
    params = std::move(obj.params);
}

URL& URL::operator=(URL&& obj){
    path = std::move(obj.path);
    params = std::move(obj.params);
    return *this;
}

void URL::Parse(const std::string & src){
    this->Clear();
    if(src.empty())
        return;
    *path = *split(src,{"\\","/"});
    if(path->empty())
        return;
    auto params_array = split(path->back(),{"?"},1);
    path->pop_back();
    path->push_back(params_array->at(0));
    if(params_array->size() <= 1){
        return;
    }
    std::string str_params = params_array->at(1);
    std::shared_ptr<std::vector<std::string>> vec_params = split(str_params,{"&","="});
    for(size_t i = 0; i < vec_params->size()-1;i+=2){
        params->insert(std::make_pair((*vec_params)[i],(*vec_params)[i+1]));
    }
}

std::vector<std::string>& URL::UrlList(){
    return *this->path;
}

std::shared_ptr<std::string> URL::PathStr() const
{
    std::shared_ptr<std::string> res = std::make_shared<std::string>("");
    for( const auto& el : *path){
        *res += "\\" + el;
    }
    return res;
}
std::map<std::string,std::string>& URL::Params(){
    return *this->params;
}

std::shared_ptr<std::string> URL::toStr()const{
    std::shared_ptr<std::string> res = std::make_shared<std::string>("");
    for( const auto& el : *path){
        *res += "\\" + el;
    }
    if(path->empty())
        res->append("\\");
    if(params->size()){
        *res += "?";
        for(const auto& el: *params)
            *res += el.first + "=" + el.second + "&";
        res->pop_back();
    }
    return res;
}

void URL::Clear(){
    path->clear();
    params->clear();
}