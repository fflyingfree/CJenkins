//
// Created by BaiFF on 2018/4/17.
//
#include "conf.h"

#ifdef CJENKINS_CONF_H

string CConf::get_cfg_env() {
    return get_cfg_tag("env");
}

string CConf::get_cfg_jHome() {
    return get_cfg_tag("jenkins_home");
}

string CConf::get_cfg_shHome() {
    return get_cfg_tag("flowshell_home");
}

string CConf::get_cfg_dbInfo() {
    string connStr=get_cfg_tag("db_info");
    return dealDBconn(connStr);
}

string CConf::get_cfg_tag(const char *tag) {
    string ret="";
    string stag=(string)tag;
    ifstream ifs;
    ifs.open("./conf/conf.cjf");
    while(!ifs.eof()){
        string line="";
        ifs >> line;
        if(line == "")continue;
        if(line.at(0) == '#')continue;
        if(line.substr(0,stag.length()) == stag){
            vector<string> v = CUtils::splitStrByChar(line,':');
            if(v.size() == 2){
                ret=v[1];
            }
            break;
        }
    }
    ifs.close();
    return ret;
}

string CConf::dealDBconn(string &str) {
    string ret="";
    vector<string> v = CUtils::splitStrByChar(str,';');
    for(int i=0;i<v.size();i++){
        if(CUtils::str2LowCase(v[i].substr(0,3).c_str()) == "pwd" ){
            vector<string> vv=CUtils::splitStrByChar(v[i],'=');
            if(vv.size() == 2){
                ret+=vv[0];
                ret+="=";
                if(vv[1].length() < 32){
                    ret+=vv[1];
                }else{
                    char* outPwd=rmsAesDecode((char*)vv[1].c_str(),vv[1].length());
                    ret+=(string)outPwd;
                    if(NULL != outPwd){
                        rmsAesFree(&outPwd);
                    }
                }
            }else{
                ret+=v[i];
            }
        }else{
            ret+=v[i];
        }
        ret+=";";
    }
    return ret;
}

#endif
