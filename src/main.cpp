#include "../headers/HttpServer.h"
#include <io.h>
#include <fcntl.h>


int main(){
    SetConsoleOutputCP(65001);
    HTTPServer* p_serv = nullptr;
    try{
        ServerSettings settings(L"mimetypes.txt",L"access.txt",L"settings.txt");
        HTTPServer s(settings);
        p_serv = &s;
        s.exec();
    }
    catch(std::exception &ex){
        std::cout << "Error: " << ex.what() << std::endl;
        if(!p_serv)
            return -1; 
        std::wstring post_log = L"Error " + fromUTF8(ex.what());
        p_serv->WriteTolog(post_log);
        return -1;
    }
    return 0;
}