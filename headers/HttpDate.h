#ifndef HTTPDATE
#define HTTPDATE
#include <Windows.h>
#include <execution>
#include <string>
#include <ctime> 
#include <sstream>
#include "../headers/OtherFunction.h"

class HttpDate{
public:
    enum dtElem{
        YEAR,
        MONTH,
        DAY,
        WDAY,
        HOUR,
        MINUTE,
        SEC,
    };
private:
    struct httpdt{
        int sec = 0, minute = 0, hour = 0, day = 0, mon = 0, year = 0, wday = 0;
    };
private:
    const std::string swday[7]{
        "Mod","Tue","Wed","Thu","Fri","Sat","Sun",
    };
    const std::string smonth[12]{
        "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec",
    };
    union{
        httpdt dt;
        int arr_dt[7];
    };
public:
    HttpDate();
    HttpDate(const std::string& src);
    HttpDate(int year,int mon,int day,int hour,int min,int sec,int wday);
    HttpDate(const HttpDate& date);
    HttpDate& operator=(const std::string& src) noexcept;
    HttpDate& operator=(const HttpDate& date) noexcept;
    friend bool operator<(const HttpDate& first,const HttpDate& second) noexcept;
    friend bool operator>(const HttpDate& first,const HttpDate& second) noexcept;
    friend bool operator==(const HttpDate& first,const HttpDate& second) noexcept;
    friend bool operator!=(const HttpDate& first,const HttpDate& second) noexcept;
    friend bool operator<=(const HttpDate& first,const HttpDate& second) noexcept;
    friend bool operator>=(const HttpDate& first,const HttpDate& second) noexcept;
    int get(dtElem elem)const noexcept;
    std::string get_str(dtElem elem)const noexcept;
    std::string toStr()const noexcept;
    void fromStr(const std::string& src);
    static HttpDate getCurGmtTime()noexcept;
    static HttpDate getFileGmtTime(const std::wstring &w_path);
};
#endif