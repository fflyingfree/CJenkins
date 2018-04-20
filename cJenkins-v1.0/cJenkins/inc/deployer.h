//
// Created by BaiFF on 2018/4/16.
//

#ifndef CJENKINS_DEPLOYER_H
#define CJENKINS_DEPLOYER_H

#include "comm.h"
#include "utils.h"
#include "conf.h"
#include <pthread.h>

class CDeployer{
public:
    int process(ST_PROGRAM &stProgram,int destID);
    vector<string> getVout();
    ST_RST getRst();
    CDeployer(){ };
    ~CDeployer(){ };
private:
    int init(ST_PROGRAM &stProgram,int destID);
    int buildExpectFile(ST_PROGRAM &stProgram);
    int preDeployShell(ST_PROGRAM &stProgram);
    int transSrcCode(ST_PROGRAM &stProgram);   //发布源码
    int scpShell(ST_PROGRAM &stProgram);
    int afterDeployShell(ST_PROGRAM &stProgram);
private:
    int exec();
    int setFlowCode(const char* chcode);
    int setErrorCode(const char* chcode);
    int analyzeRst();    //分析执行结果
private:
    ST_HOST* stDestHost;
private:
    string expectUrl;
    string execShell1,execShell2,execShell3,execShell4;
    ofstream expectFile;   //发布全流程脚本
private:
    int FlowArr[7];
    int ErrorArr[7];
    vector<string> vOut;
    ST_RST stRst;
};

class CMulDeployer{  //并行发布到目标主机
public:
    static int deploy(ST_PROGRAM &stProgram);
    static vector<ST_RST> getVrsts();
private:
    static void* run(void* para);
    CMulDeployer(){ };
    ~CMulDeployer(){ };
private:
    static int pushPout(vector<string> p_vout);
    static int printOut();
    static int pushRst(ST_RST &rst);
private:
    static vector<vector<string> > v_v_outs;
    static vector<ST_RST> vRsts;
private:
    typedef struct{
        vector<int> taskID;
        ST_PROGRAM stProgram;
    }ST_PARA;
};

#endif //CJENKINS_DEPLOYER_H
