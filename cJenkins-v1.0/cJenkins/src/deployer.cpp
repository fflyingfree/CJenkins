//
// Created by BaiFF on 2018/4/16.
//

#include "deployer.h"
#include "comm.h"

#ifdef CJENKINS_DEPLOYER_H

int CDeployer::process(ST_PROGRAM &stProgram,int destID) {
    this->init(stProgram,destID);
    this->buildExpectFile(stProgram);
    this->exec();
    if(this->analyzeRst() == 0){
        return 0;  //成功
    }
    return 2; //失败
}

int CDeployer::init(ST_PROGRAM &stProgram,int destID) {
    this->stDestHost=&(stProgram.stDeployInfo.v_stDeployHost[destID]);

    string JenkinsHome = CConf::get_cfg_jHome();    //从配置文件获取
    string shHome = JenkinsHome+CConf::get_cfg_shHome();
    if(!CUtils::ifExist(shHome.c_str())){
        string cmd="mkdir -p "+shHome+"compile/";
        cmd+=";";
        cmd+=("mkdir -p "+shHome+"deploy/");
        system(cmd.c_str());
    }
    char pid_buf[100]={0};
    sprintf(pid_buf,"%d",destID/*getpid()*/);
    this->expectUrl=shHome+"deploy/expect_"+stProgram.programName+"."+(string)pid_buf;
    this->execShell1=shHome+"deploy/exec_shell1_"+stProgram.programName+"."+(string)pid_buf;
    this->execShell2=shHome+"deploy/exec_shell2_"+stProgram.programName+"."+(string)pid_buf;
    this->execShell3=shHome+"deploy/exec_shell3_"+stProgram.programName+"."+(string)pid_buf;
    this->execShell4=shHome+"deploy/exec_shell4_"+stProgram.programName+"."+(string)pid_buf;

    for(int i=0;i<7;++i){
        this->FlowArr[i]=0;
        this->ErrorArr[i]=0;
    }

    this->stRst.programName=stProgram.programName;
    this->stRst.hostIP=this->stDestHost->hostIP;
    this->stRst.stage="";
    this->stRst.reason="";

    return 0;
}

int CDeployer::buildExpectFile(ST_PROGRAM &stProgram) {
    this->expectFile.open(this->expectUrl.c_str(),ios::trunc);
    this->expectFile.close();
    this->expectFile.open(this->expectUrl.c_str(),ios::ate | ios::app);
    this->expectFile << "#!/usr/bin/expect" << endl;
    //this->expectFile << "set timeout -1" << endl;
    this->expectFile << "set timeout 33" << endl;

    this->preDeployShell(stProgram);
    this->scpShell(stProgram);
    this->afterDeployShell(stProgram);

    this->expectFile << "puts \"==expect(:0)\"" << endl;     //标识整个脚本执行到最后
    this->expectFile.flush();
    this->expectFile.close();
    return 0;
}

int CDeployer::preDeployShell(ST_PROGRAM &stProgram) {
    string user=this->stDestHost->hostUser;
    string ip=this->stDestHost->hostIP;
    string pwd=this->stDestHost->hostPwd;
    int modnum=stProgram.v_stModule.size();

    this->expectFile << "spawn /usr/bin/ssh " << user << "@" << ip << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"*password:\" {send \"" << pwd << "\\r\"}" << endl;
    this->expectFile << "\"*]$ \" {send \"\\r\"}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"*password:\" {puts \"==expect(:-1)\";exit}" << endl;
    this->expectFile << "\"*]$ \" {send \"\\r\"}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "puts \"==expect(:1)\"; send \"\\r\"" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;

    ofstream ofs;
    if(stProgram.stDeployInfo.preShell != ""){
        ofs.open(this->execShell1.c_str());
        ofs << stProgram.stDeployInfo.preShell << "; " << endl;
        ofs << "if [ $? -ne 0 ]; then echo \"F\";echo \"==exec(:2)\"; else echo \"F\";echo \"==exec(:0)\";fi ";
        ofs << "\r";
        ofs.flush();
        ofs.close();

        this->expectFile << "send [read [open " << this->execShell1 << " r] ]" << endl;
        this->expectFile << "expect {" << endl;
        this->expectFile << "\"F==exec(:0)*]$ \" {puts \"==expect(:2)\"; send \"\\r\"}" << endl;
        this->expectFile << "\"F==exec(:2)*]$ \" {puts \"==expect(:-2)\"; exit}" << endl;
        this->expectFile << "\"*]$ \" {puts \"==expect(:-2)\"; exit}" << endl;
        this->expectFile << "}" << endl;
    }else{
        this->expectFile << "puts \"==expect(:2)\"; send \"\\r\"" << endl;
    }
    this->expectFile << "expect \"*]$ \"" << endl;

    ofs.open(this->execShell2.c_str());
    for(int i=0;i<modnum;i++){
        vector<string> vurl= CUtils::splitStrByChar(stProgram.v_stModule[i].deployUrl, ';');
        vector<string> vpath=CUtils::splitStrByChar(stProgram.v_stModule[i].deployPath,';');
        if(vurl.size() < 1 || vurl.size() != vpath.size()){
            cerr << "表配置错误" << endl;
            continue;
        }
        for(int j=0;j<vurl.size();j++){
            vector<string> vName=CUtils::splitStrByChar(vurl[j],'/');
            ofs << "x=0; ";
            ofs << "(cd " << vpath[j];
            ofs << " || ";
            ofs << "mkdir -p " << vpath[j] << "); ";
            ofs << "if [ $? -ne 0 ]; then echo \"F\";echo \"==exec(:2)\";x=2; else cd " << vpath[j] << "; fi; ";
            ofs << "if [ $x -ne 2 ]; then ";
            ofs << "ls " << vName[vName.size()-1] << "; ";
            ofs << "if [ $? -ne 0 ]; then x=2; fi; ";
            ofs << "fi; ";
            ofs << "if [ $x -ne 2 ]; then ";
            ofs << "mv " << vName[vName.size()-1] << " " << vName[vName.size()-1] << "_`date +%Y%m%d%H%M%S`.bakk" << "; ";
            ofs << "if [ $? -ne 0 ]; then echo \"F\";echo \"==exec(:2)\";x=2; fi; ";
            ofs << "fi; ";
            ofs << "if [ $x -ne 2 ]; then ";
            ofs << "num=0;num=`ls -lrt | grep " << vName[vName.size()-1] << "_ | grep .bakk | wc -l`;if [ $num -gt 3 ]; then y=`expr $num - 3`;ls -tr | grep " << vName[vName.size()-1] << "_ | grep .bakk | head -$y | xargs rm -rf { }; if [ $? -ne 0 ]; then echo -n \"F\";echo -n \"==exec(:2)\";x=2; fi; fi; ";
            ofs << "fi";

            if(j != vurl.size()-1){
                ofs << ";";
            }
            ofs << " ";
        }
    }
    ofs << "\r";
    ofs.flush();
    ofs.close();

    this->expectFile << "send [read [open " << this->execShell2 << " r] ]" << endl;
    this->expectFile << "expect { " << endl;
    this->expectFile << "\"F==exec(:2)*]$ \" {puts \"==expect(:-3)\"; exit}" << endl;
    this->expectFile << "\"*]$ \" {send \"\\r\"}" << endl;
    this->expectFile << "} " << endl;
    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "puts \"==expect(:3)\";send \"\\r\"" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;

    if(stProgram.stDeployInfo.ifDeploySrc==1 && this->stDestHost->hostIP != stProgram.stCompileInfo.stCompileHost.hostIP){  //同时发布源码
        this->transSrcCode(stProgram);
    }else{
        this->expectFile << "puts \"==expect(:4)\";send \"\\r\"" << endl;
        this->expectFile << "expect \"*]$ \"" << endl;
    }

    return 0;
}

int CDeployer::transSrcCode(ST_PROGRAM &stProgram) {
    string user=this->stDestHost->hostUser;
    string ip=this->stDestHost->hostIP;
    string pwd=this->stDestHost->hostPwd;
    string srcUser=stProgram.stCompileInfo.stCompileHost.hostUser;
    string srcIP=stProgram.stCompileInfo.stCompileHost.hostIP;
    string srcPwd=stProgram.stCompileInfo.stCompileHost.hostPwd;
    string srcPath=stProgram.stCompileInfo.compileSrcPath;  //发布主机、编译主机源码路径一致
    string srcDirectoryName=stProgram.programName;    //源码文件夹名 同 程序名
    ofstream ofs;
    ofs.open(this->execShell3.c_str());
    string cmd="";
    cmd+=("x=0; ");
    cmd+=("cd "+srcPath+"; ");
    cmd+=("if [ $? -ne 0 ]; then mkdir -p "+srcPath+"; fi; ");
    ofs << cmd;
    cmd="";
    cmd+=("cd "+srcPath+"; ");
    cmd+=("if [ $? -ne 0 ]; then echo -n \"F\";echo -n \"==exec(:2)\";x=2; fi; ");
    ofs << cmd;
    cmd="";
    cmd+=("if [ $x -ne 2 ]; then ");
    cmd+=("ls "+srcDirectoryName+"; ");
    cmd+=("if [ $? -ne 0 ]; then echo -n \"F\";echo -n \"==exec(:0)\";x=2; fi; ");
    cmd+=("fi; ");
    ofs << cmd;
    cmd="";
    cmd+=("if [ $x -ne 2 ]; then ");
    cmd+=("mv "+srcDirectoryName+" "+srcDirectoryName+"_`date +%Y%m%d%H%M%S`.bakk; ");
    cmd+=("if [ $? -ne 0 ]; then echo -n \"F\";echo -n \"==exec(:2)\";x=2; fi; ");
    cmd+=("fi; ");
    ofs << cmd;
    cmd="";
    cmd+=("if [ $x -ne 2 ]; then ");
    cmd+=("num=0;num=`ls -lrt | grep "+srcDirectoryName+"_ | grep .bakk | wc -l`;if [ $num -gt 3 ]; then y=`expr $num - 3`;ls -tr | grep "+srcDirectoryName+"_ | grep .bakk | head -$y | xargs rm -rf { }; if [ $? -ne 0 ]; then echo -n \"F\";echo -n \"==exec(:2)\";x=2; fi; fi; ");
    cmd+=("fi; ");
    ofs << cmd;
    cmd="";
    cmd+=("if [ $x -ne 2 ]; then ");
    cmd+=("echo -n \"F\";echo -n \"==exec(:0)\"; ");
    cmd+=("fi ");
    ofs << cmd;
    ofs << "\r";
    ofs.flush();
    ofs.close();
    this->expectFile << "send [read [open " << this->execShell3 << " r] ]" << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"F==exec(:0)*]$ \" {send \"\\r\"}" << endl;
    this->expectFile << "\"F==exec(:2)*]$ \" {puts \"==expect(:-4)\"; exit}" << endl;
    this->expectFile << "\"*]$ \" {puts \"==expect(:-4)\"; exit}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;

    this->expectFile << "set ifflg 0" << endl;
    this->expectFile << "send \"scp -r " << srcUser << "@" << srcIP << ":" << srcPath << srcDirectoryName << " " << srcPath << "\\r\"" << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"(yes/no)?\" {" << endl;
    this->expectFile << "send \"yes\\r\"" << endl;
    this->expectFile << "expect \"*password:\" {set ifflg 1;send \"" << srcPwd << "\\r\"}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "\"*password:\" {set ifflg 1;send \"" << srcPwd << "\\r\"}" << endl;
    this->expectFile << "\"100%\" {set ifflg 0;puts \"==expect(:4)\"; send \"\\r\"}" << endl;
    this->expectFile << "\"*]$ \" {puts \"==expect(:-4)\"; exit}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "if { $ifflg == 1 } { " << endl;
    this->expectFile << "\texpect {" << endl;
    this->expectFile << "\t\"100%\" {puts \"==expect(:4)\"; send \"\\r\"}" << endl; //能看到100% 说明scp成功  否则失败
    this->expectFile << "\t\"*]$ \" {puts \"==expect(:-4)\"; exit}" << endl;
    this->expectFile << "\t}" << endl;
    this->expectFile << "} " << endl;
    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "send \"logout\\r\"" << endl;
    this->expectFile << "expect eof" << endl;
    this->expectFile << "spawn /usr/bin/ssh " << user << "@" << ip << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"*password:\" {send \"" << pwd << "\\r\"}" << endl;
    this->expectFile << "\"*]$ \" {send \"\\r\"}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;

    return 0;
}

int CDeployer::scpShell(ST_PROGRAM &stProgram) {
    int modnum = stProgram.v_stModule.size();
    string srcIP=stProgram.stCompileInfo.stCompileHost.hostIP;
    string srcUser=stProgram.stCompileInfo.stCompileHost.hostUser;
    string srcPwd=stProgram.stCompileInfo.stCompileHost.hostPwd;

    for(int i=0;i<modnum;i++){
        vector<string> vurl= CUtils::splitStrByChar(stProgram.v_stModule[i].deployUrl, ';');
        vector<string> vpath=CUtils::splitStrByChar(stProgram.v_stModule[i].deployPath,';');
        if(vurl.size() < 1 || vurl.size() != vpath.size()){
            cerr << "表配置错误" << endl;
            continue;
        }
        for(int j=0;j<vurl.size();j++){
            string srcUrl=vurl[j];   //修改配置表配置为绝对路径   源主机和目标主机密码输入判断？
            string destPath=vpath[j];
            this->expectFile << "set ifflg 0" << endl;
            this->expectFile << "send \"scp " << srcUser << "@" << srcIP << ":" << srcUrl << " " << destPath << "\\r\"" << endl;
            this->expectFile << "expect {" << endl;
            this->expectFile << "\"(yes/no)?\" {" << endl;
            this->expectFile << "send \"yes\\r\"" << endl;
            this->expectFile << "expect \"*password:\" {set ifflg 1;send \"" << srcPwd << "\\r\"}" << endl;
            this->expectFile << "}" << endl;
            this->expectFile << "\"*password:\" {set ifflg 1;send \"" << srcPwd << "\\r\"}" << endl;
            this->expectFile << "\"100%\" {set ifflg 0;puts \"==expect(:5)\"; send \"\\r\"}" << endl;
            this->expectFile << "\"*]$ \" {puts \"==expect(:-5)\"; exit}" << endl;
            this->expectFile << "}" << endl;
            this->expectFile << "if { $ifflg == 1 } { " << endl;
            this->expectFile << "\texpect {" << endl;
            this->expectFile << "\t\"100%\" {puts \"==expect(:5)\"; send \"\\r\"}" << endl;
            this->expectFile << "\t\"*]$ \" {puts \"==expect(:-5)\"; exit}" << endl;
            this->expectFile << "\t}" << endl;
            this->expectFile << "} " << endl;
        }
    }

    return 0;
}

int CDeployer::afterDeployShell(ST_PROGRAM &stProgram) {
    this->expectFile << "expect \"*]$ \"" << endl;

    if(stProgram.stDeployInfo.afterShell != ""){
        ofstream ofs;
        ofs.open(this->execShell4.c_str());
        ofs << stProgram.stDeployInfo.afterShell << "; " << endl;
        ofs << "if [ $? -ne 0 ]; then echo \"F\";echo \"==exec(:2)\"; else echo \"F\";echo \"==exec(:0)\";fi ";
        ofs << "\r";
        ofs.flush();
        ofs.close();

        this->expectFile << "send [read [open " << this->execShell4 << " r] ]" << endl;
        this->expectFile << "expect {" << endl;
        this->expectFile << "\"F==exec(:0)*]$ \" {puts \"==expect(:6)\"; send \"\\r\"}" << endl;
        this->expectFile << "\"F==exec(:2)*]$ \" {puts \"==expect(:-6)\"; exit}" << endl;
        this->expectFile << "\"*]$ \" {puts \"==expect(:-6)\"; exit}" << endl;
        this->expectFile << "}" << endl;
    }else{
        this->expectFile << "puts \"==expect(:6)\"; send \"\\r\"" << endl;
    }

    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "send \"logout\\r\"" << endl;
    this->expectFile << "expect eof" << endl;
    return 0;
}

int CDeployer::exec() {
    FILE* fp = NULL;
    string cmd = "expect "+this->expectUrl;
    char buf[1024]={0};
    if((fp=popen(cmd.c_str(),"r"))!=NULL){
        this->vOut.clear();
        while(fgets(buf,sizeof(buf),fp)!=NULL){
            string sbuf = (string)buf;
            this->vOut.push_back(sbuf);  //由外部程序输出
            memset(buf,0x00, sizeof(buf));
            if(strstr(sbuf.c_str(),"==expect(:") !=NULL){
                for(int i=0;i<sbuf.length();++i){
                    if(sbuf.substr(i,10)=="==expect(:"){
                        int flg=0;
                        int loc=i+10;
                        if(loc < sbuf.length() && sbuf.at(loc)=='-'){
                            loc+=1;
                            flg=1;
                        }
                        string code="";  //标识码
                        for(int j=loc;j<sbuf.length();++j){
                            if(j < sbuf.length() && sbuf.at(j)==')'){
                                code=sbuf.substr(loc,j-loc);
                                if(flg == 1){  //错误码
                                    this->setErrorCode(code.c_str());
                                }else{  //流程码
                                    this->setFlowCode(code.c_str());
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
        pclose(fp);
    }

    return 0;
}

int CDeployer::setFlowCode(const char *chcode) {
    int code = atoi(chcode);
    this->FlowArr[code]=1;
    return 0;
}

int CDeployer::setErrorCode(const char *chcode) {
    int code = atoi(chcode);
    this->ErrorArr[code]=1;
    return 0;
}

int CDeployer::analyzeRst() {
    if(this->FlowArr[0] == 1){  //全流程脚本执行到结尾
        int sumE=0;
        for(int i=1;i<7;i++){
            sumE+=this->ErrorArr[i];
        }
        if(sumE == 0){
            cout << "发布成功+1" << endl;   //一台发布主机
            this->stRst.stage="SUCCESS✔";
            this->stRst.reason="...";
            return 0;
        }
    }

    this->stRst.stage="FAIL✘";
    int fi=0;
    for(int i=7;i>1;i--){
        if(this->ErrorArr[i-1]==1){
            fi=i;
            break;
        }
    }
    switch(fi){
        case 1:
            this->stRst.reason="ssh登陆发布主机失败";
            break;
        case 2:
            this->stRst.reason="发布主机发布前处理失败";
            break;
        case 3:
            this->stRst.reason="检查发布主机发布路径-程序备份-历史程序清理失败";
            break;
        case 4:
            this->stRst.reason="scp源码失败";
            break;
        case 5:
            this->stRst.reason="scp程序失败";
            break;
        case 6:
            this->stRst.reason="发布主机发布后处理失败";
            break;
        default:
            this->stRst.reason="unknown";
    }

    return -1; //出错
}

vector<string> CDeployer::getVout() {
    return this->vOut;
}

ST_RST CDeployer::getRst() {
    return this->stRst;
}

/*
 * **********###########**************############**************#############***************##############*************###############*************
 */

//##MulDeployer##//      多线程并行发布

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2=PTHREAD_MUTEX_INITIALIZER;
vector<vector<string> > CMulDeployer::v_v_outs;
vector<ST_RST> CMulDeployer::vRsts;

int CMulDeployer::deploy(ST_PROGRAM &stProgram) {
    v_v_outs.clear();
    vRsts.clear();

    int threadNum = stProgram.stDeployInfo.v_stDeployHost.size() < 50 ? stProgram.stDeployInfo.v_stDeployHost.size() : 50;
    int unitTaskNum = stProgram.stDeployInfo.v_stDeployHost.size() / threadNum;
    int otherTaskNum = stProgram.stDeployInfo.v_stDeployHost.size() % threadNum;

    pthread_t tids[threadNum];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    for(int i=0;i<threadNum;i++){
        vector<int> vTaskID;
        vTaskID.clear();
        int tmp=0;
        tmp=i*unitTaskNum;
        for(int ii=0;ii<unitTaskNum;ii++){
            vTaskID.push_back(tmp+ii);
        }
        if(i<otherTaskNum){
            tmp=(stProgram.stDeployInfo.v_stDeployHost.size()-otherTaskNum)+i;
            vTaskID.push_back(tmp);
        }
        ST_PARA stPara;
        stPara.taskID=vTaskID;
        stPara.stProgram=stProgram;
        pthread_create(&tids[i],NULL,run,(void*)(&stPara));
        usleep(100);
    }
    pthread_attr_destroy(&attr);
    for(int i=0;i<threadNum;i++){
        void* status;
        pthread_join(tids[i],&status);
    }
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    printOut();
    return 0;
}

void* CMulDeployer::run(void *para) {
    ST_PARA* stPara = (ST_PARA*)para;
    for(int i=0;i<stPara->taskID.size();i++){
        CDeployer cDeployer;
        cDeployer.process(stPara->stProgram,stPara->taskID[i]);
        vector<string> p_vout = cDeployer.getVout();
        pushPout(p_vout);
        ST_RST rst = cDeployer.getRst();
        pushRst(rst);
    }
}

int CMulDeployer::pushPout(vector<string> p_vout) {
    pthread_mutex_lock(&mutex);
    v_v_outs.push_back(p_vout);
    pthread_mutex_unlock(&mutex);
    return 0;
}

int CMulDeployer::printOut() {
    cout << "##########################################################################################\r\n" << endl;
    vector<vector<string> >::iterator it;
    for(it=v_v_outs.begin();it!=v_v_outs.end();++it){
        for(int i=0;i<(*it).size();++i){
            cout << (*it)[i];
        }
        cout << "##########################################################################################" << endl;
    }
    return 0;
}

int CMulDeployer::pushRst(ST_RST &rst) {
    pthread_mutex_lock(&mutex2);
    vRsts.push_back(rst);
    pthread_mutex_unlock(&mutex2);
    return 0;
}

vector<ST_RST> CMulDeployer::getVrsts() {
    return vRsts;
}

#endif
