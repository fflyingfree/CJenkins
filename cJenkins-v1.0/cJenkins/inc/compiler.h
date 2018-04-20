//
// Created by BaiFF on 2018/4/12.
//

#ifndef CJENKINS_COMPILER_H
#define CJENKINS_COMPILER_H

#include "comm.h"
#include "conf.h"

class CCompiler{
public:
    int process(ST_PROGRAM &stProgram);
    ST_RST getRst();
    CCompiler(){ };
    ~CCompiler(){ };
private:
    int init(ST_PROGRAM &stProgram);
    int buildExpectFile(ST_PROGRAM &stProgram);
    int sureSrcPathShell(ST_PROGRAM &stProgram);
    int scpSrcShell(ST_PROGRAM &stProgram);
    int modulesMakeShell(ST_PROGRAM &stProgram);
private:
    int exec();
    int setFlowCode(const char* chcode);
    int setErrorCode(const char* chcode);
    int analyzeRst();    //分析执行结果
private:
    string localSrcUrl;
    string expectUrl;
    string execShell1,execShell2;
    ofstream expectFile;   //编译全流程脚本
private:
    int FlowArr[5];
    int ErrorArr[5];
    ST_RST stRst;
};

#endif //CJENKINS_COMPILER_H
