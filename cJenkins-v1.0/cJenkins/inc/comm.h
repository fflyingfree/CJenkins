//
// Created by BaiFF on 2018/4/12.
//

#ifndef CJENKINS_COMM_H
#define CJENKINS_COMM_H

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

typedef struct{
    string modName;
    string compileExecPath;
    string compileCmd;
    string deployUrl;
    string deployPath;
}ST_MODULE;

typedef struct{
    string hostIP;
    string hostUser;
    string hostPwd;
}ST_HOST;

typedef struct{
    ST_HOST stCompileHost;
    string compileSrcPath;
}ST_COMPILE;

typedef struct{
    vector<ST_HOST> v_stDeployHost;
    string preShell;
    string afterShell;
    int ifDeploySrc;
}ST_DEPLOY;

typedef struct{
    string programName;
    ST_COMPILE stCompileInfo;
    ST_DEPLOY stDeployInfo;
    vector<ST_MODULE> v_stModule;
}ST_PROGRAM;

typedef struct{
    string groupName;
    vector<ST_PROGRAM> v_stProgram;
}ST_GROUP;

typedef struct{
    string programName;
    string hostIP;         //编译 or 发布主机
    string stage;          //状态
    string reason;         //原因
}ST_RST;

typedef struct{
    string programName;
    string cHost;
    string dHost;
    string cRst;
    string dRst;
    string rst;
    string rstDesc;
}ST_RECORD;

#endif //CJENKINS_COMM_H
