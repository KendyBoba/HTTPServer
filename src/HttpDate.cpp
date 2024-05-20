#include "../headers/HttpDate.h"

HttpDate::HttpDate()
{

}

HttpDate::HttpDate(const std::string &src)
{
    fromStr(src);
}

HttpDate::HttpDate(int year, int mon, int day, int hour, int minute, int sec,int wday):
    dt{sec,minute,hour,day,mon,year,wday}
{
}

HttpDate::HttpDate(const HttpDate &date)
{
    for(short i = 0; i < 7;++i)
        this->arr_dt[i] = date.arr_dt[i];
}

HttpDate &HttpDate::operator=(const std::string &src) noexcept
{
    fromStr(src);
    return *this;
}

HttpDate &HttpDate::operator=(const HttpDate &date) noexcept
{
    for(short i = 0; i < 7;++i)
        this->arr_dt[i] = date.arr_dt[i];
    return *this;
}

std::string HttpDate::toStr() const noexcept
{
    std::stringstream ss;
    ss << " " << get_str(WDAY) << ", " << get_str(DAY) 
    << " " << get_str(MONTH) <<  " " << get_str(YEAR) 
    << " " << get_str(HOUR) << ":" 
    << get_str(MINUTE) << ":" << get_str(SEC) 
    << " " << "GMT";
    return ss.str();
}

void HttpDate::fromStr(const std::string &src)
{
   auto split_dt = split(src,{","," "});
   std::string str_wday = trim(split_dt->at(0));
   for(uint8_t i = 0; i < 7;++i){
        if(str_wday == swday[i])
            this->dt.wday = i;
   }
   dt.day = std::stoi(trim(split_dt->at(1)));
   std::string smon = trim(split_dt->at(2));
   for(uint8_t i = 0; i < 12;++i){
        if(smon == this->smonth[i])
            dt.mon = i;
   }
   dt.year = stoi(trim(split_dt->at(3)));
   auto split_time = split(trim(split_dt->at(4)),{":"});
   dt.hour = stoi(split_time->at(0));
   dt.minute = stoi(split_time->at(1));
   dt.sec = stoi(split_time->at(2));
}

HttpDate HttpDate::getCurGmtTime() noexcept
{
    HttpDate res;
    time_t cur_time = time(nullptr);
    tm gmt_dt = *gmtime(&cur_time);
    for(uint8_t i = 0; i < 7;++i)
        res.arr_dt[i] = (reinterpret_cast<int*>(&gmt_dt))[i];
    res.dt.year += 1900;
    return res;
}

HttpDate HttpDate::getFileGmtTime(const std::wstring &w_path)
{
    HttpDate res;
    WIN32_FIND_DATAW wfd;
    HANDLE h_file = FindFirstFileW(w_path.c_str(),&wfd);
    if(h_file == INVALID_HANDLE_VALUE)
        throw std::domain_error("file was not found");
    FindClose(h_file);
    SYSTEMTIME sys_time;
    FileTimeToSystemTime(&wfd.ftLastWriteTime,&sys_time);
    tm gmt_dt;
    gmt_dt.tm_year = sys_time.wYear - 1900;
    gmt_dt.tm_mon = sys_time.wMonth;
    gmt_dt.tm_mday = sys_time.wDay;
    gmt_dt.tm_hour = sys_time.wHour;
    gmt_dt.tm_min = sys_time.wMinute;
    gmt_dt.tm_sec = sys_time.wSecond;
    std::time_t time = std::mktime(&gmt_dt);
    gmt_dt = *std::gmtime(&time);
    for(uint8_t i = 0; i < 7;++i)
        res.arr_dt[i] = (reinterpret_cast<int*>(&gmt_dt))[i];
    res.dt.year += 1900;
    return res;
}

int HttpDate::get(dtElem elem) const noexcept
{
    switch (elem)
    {
    case dtElem::YEAR: return this->dt.year; break;
    case dtElem::MONTH: return this->dt.mon; break;
    case dtElem::DAY: return this->dt.day; break;
    case dtElem::HOUR: return this->dt.hour; break;
    case dtElem::MINUTE: return this->dt.minute; break;
    case dtElem::SEC: return this->dt.sec; break;
    case dtElem::WDAY: return this->dt.wday; break;
    }
    return -1;
}

std::string HttpDate::get_str(dtElem elem) const noexcept
{
    switch (elem)
    {
    case dtElem::YEAR:{
        return std::to_string(this->dt.year);
    };break;
    case dtElem::MONTH:{
        return smonth[this->dt.mon];
    };break;
    case dtElem::DAY:{
        std::string str_day = std::to_string(this->dt.day);
        return (this->dt.day <= 9) ? "0" + str_day: str_day;
    };break;
    case dtElem::HOUR:{
        std::string str_hour = std::to_string(this->dt.hour);
        return (this->dt.hour <= 9) ? "0" + str_hour: str_hour;
    };break;
    case dtElem::MINUTE:{
        std::string str_min = std::to_string(this->dt.minute);
        return (this->dt.minute <= 9) ? "0" + str_min: str_min;
    };break;
    case dtElem::SEC:{
        std::string str_sec = std::to_string(this->dt.sec);
        return (this->dt.sec <= 9) ? "0" + str_sec: str_sec;
    };break;
    case dtElem::WDAY:{
        return swday[this->dt.wday];
    };break;
    }
    return std::string();
}

bool operator<(const HttpDate &first, const HttpDate &second) noexcept
{
    for(uint8_t i = 6; i >= 0;--i){
        if(first.arr_dt[i] < second.arr_dt[i])
            return true;
        else if(first.arr_dt[i] == second.arr_dt[i])
            continue;
        else
            return false;
    }
    return false;
}

bool operator>(const HttpDate &first, const HttpDate &second) noexcept
{
    for(uint8_t i = 6; i >= 0;--i){
        if(first.arr_dt[i] > second.arr_dt[i])
            return true;
        else if(first.arr_dt[i] == second.arr_dt[i])
            continue;
        else
            return false;
    }
    return false;
}

bool operator==(const HttpDate &first, const HttpDate &second) noexcept
{
    for(uint8_t i = 6; i >= 0;--i){
        if(first.arr_dt[i] != second.arr_dt[i])
            return false;
    }
    return true;
}

bool operator!=(const HttpDate &first, const HttpDate &second) noexcept
{
    return !(first == second);
}

bool operator<=(const HttpDate &first, const HttpDate &second) noexcept
{
    return !(first > second);
}

bool operator>=(const HttpDate &first, const HttpDate &second) noexcept
{
    return !(first < second);
}
