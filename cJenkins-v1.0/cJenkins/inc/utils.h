//
// Created by BaiFF on 2018/4/16.
//

#ifndef CJENKINS_UTILS_H
#define CJENKINS_UTILS_H

#include <string>
#include <vector>
#include <sys/io.h>

class CUtils{
public:
    static std::vector<std::string> splitStrByChar(std::string &str, int splitChar);
    static std::string str2LowCase(const char* cstr);
    static bool ifExist(const char* url);
private:
    CUtils(){ };
    ~CUtils(){ };
};

#endif //CJENKINS_UTILS_H
