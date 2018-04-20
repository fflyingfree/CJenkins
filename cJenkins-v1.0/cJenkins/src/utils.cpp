//
// Created by BaiFF on 2018/4/16.
//

#include "utils.h"

#ifdef CJENKINS_UTILS_H

//以字符分割字符串
std::vector<std::string> CUtils::splitStrByChar(std::string &str, int splitChar){
    std::vector<std::string> vStrs;
    vStrs.clear();
    std::string subStr;

    int len=str.length();
    int index=0;
    for(int i=0;i<len;++i){
        if(str.at(i)==splitChar){
            subStr=str.substr(index,i-index);
            vStrs.push_back(subStr);
            index=i+1;
        }
    }

    if(index < len){
        subStr=str.substr(index,len-index);
        vStrs.push_back(subStr);
    }

    return vStrs;
}

//字符串转为小写字母
std::string CUtils::str2LowCase(const char* cstr){
    std::string str=(std::string)cstr;
    for(std::string::iterator it=str.begin();it!=str.end();++it){
        if(*it > 64 && *it < 91){   //65~90之间为大写字母
            *it+=32;
        }
    }
    return str;
}

bool CUtils::ifExist(const char* url) {
    if((access(url,0)) != -1){
        return true;
    }
    return false;
}

#endif

