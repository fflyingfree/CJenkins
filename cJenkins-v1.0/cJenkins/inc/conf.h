//
// Created by BaiFF on 2018/4/17.
//

#ifndef CJENKINS_CONF_H
#define CJENKINS_CONF_H

#include <string>
#include <vector>
#include <fstream>
#include "utils.h"
#include "rmsAes.h"
using std::string;
using std::vector;
using std::ifstream;

class CConf{
public:
    static string get_cfg_env();
    static string get_cfg_jHome();
    static string get_cfg_shHome();
    static string get_cfg_dbInfo();
private:
    static string get_cfg_tag(const char* tag);
    static string dealDBconn(string &str);
    CConf(){ };
    ~CConf(){ };
};

#endif //CJENKINS_CONF_H
