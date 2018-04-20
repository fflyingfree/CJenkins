//
// Created by BaiFF on 2018/4/12.
//

#include "compiler.h"
#include "comm.h"

#ifdef CJENKINS_COMPILER_H

int CCompiler::process(ST_PROGRAM &stProgram) {
    this->init(stProgram);
    this->buildExpectFile(stProgram);
    this->exec();
    if(this->analyzeRst()==0){
        return 0;   //成功
    }
    return 2;  //失败
}

int CCompiler::init(ST_PROGRAM &stProgram) {
    string JenkinsHome = CConf::get_cfg_jHome();    //从配置文件获取
    string shHome = JenkinsHome+CConf::get_cfg_shHome();
    if(!CUtils::ifExist(shHome.c_str())){
        string cmd="mkdir -p "+shHome+"compile/";
        cmd+=";";
        cmd+=("mkdir -p "+shHome+"deploy/");
        system(cmd.c_str());
    }
    char pid_buf[100]={0};
    sprintf(pid_buf,"%d",0/*getpid()*/);
    this->expectUrl=shHome+"compile/expect_"+stProgram.programName+"."+(string)pid_buf;
    this->localSrcUrl=JenkinsHome+"workspace/"+stProgram.programName;
    this->execShell1=shHome+"compile/exec_shell1_"+stProgram.programName+"."+(string)pid_buf;
    this->execShell2=shHome+"compile/exec_shell2_"+stProgram.programName+"."+(string)pid_buf;

    for(int i=0;i<5;i++){
        this->FlowArr[i]=0;
        this->ErrorArr[i]=0;
    }

    this->stRst.programName=stProgram.programName;
    this->stRst.hostIP=stProgram.stCompileInfo.stCompileHost.hostIP;
    this->stRst.stage="";
    this->stRst.reason="";

    return 0;
}

//全流程正常返回 ==expect[:$ret] ret==0
//ret正值代表成功执行到的流程节点 1:登陆编译主机成功 2:备份编译主机源码成功 3:scp源码到编译主机成功 4:编译成功
//ret负值代表出错的流程节点 -1:备份源码失败 -2:编译命令执行路径不存在 -3:编译失败
int CCompiler::buildExpectFile(ST_PROGRAM &stProgram) {
    this->expectFile.open(this->expectUrl.c_str(),ios::trunc);
    this->expectFile.close();
    this->expectFile.open(this->expectUrl.c_str(),ios::ate | ios::app);
    this->expectFile << "#!/usr/bin/expect" << endl;
    //this->expectFile << "set timeout -1" << endl;
    this->expectFile << "set timeout 33" << endl;

    this->sureSrcPathShell(stProgram);
    this->scpSrcShell(stProgram);
    this->modulesMakeShell(stProgram);

    this->expectFile << "puts \"==expect(:0)\"" << endl;     //标识整个脚本执行到最后
    this->expectFile.flush();
    this->expectFile.close();
    return 0;
}

int CCompiler::sureSrcPathShell(ST_PROGRAM &stProgram) {
    string user=stProgram.stCompileInfo.stCompileHost.hostUser;
    string ip=stProgram.stCompileInfo.stCompileHost.hostIP;
    string pwd=stProgram.stCompileInfo.stCompileHost.hostPwd;
    string srcPath=stProgram.stCompileInfo.compileSrcPath;
    string srcDirectoryName=stProgram.programName;    //源码文件夹名 同 程序名
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
    ofs.open(this->execShell1.c_str());
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

    this->expectFile << "send [read [open " << this->execShell1 << " r] ]" << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"F==exec(:0)*]$ \" {puts \"==expect(:2)\"; send \"\\r\"}" << endl;
    this->expectFile << "\"F==exec(:2)*]$ \" {puts \"==expect(:-2)\"; exit}" << endl;
    this->expectFile << "\"*]$ \" {puts \"==expect(:-2)\"; exit}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "send \"logout\\r\"" << endl;
    this->expectFile << "expect eof" << endl;
    return 0;
}

int CCompiler::scpSrcShell(ST_PROGRAM &stProgram) {
    string user=stProgram.stCompileInfo.stCompileHost.hostUser;
    string ip=stProgram.stCompileInfo.stCompileHost.hostIP;
    string pwd=stProgram.stCompileInfo.stCompileHost.hostPwd;
    string srcPath=stProgram.stCompileInfo.compileSrcPath;
    this->expectFile << "set ifflg 0" << endl;
    this->expectFile << "spawn scp -r " << this->localSrcUrl << " " << user << "@" << ip << ":" << srcPath << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"(yes/no)?\" {" << endl;
    this->expectFile << "send \"yes\\r\"" << endl;
    this->expectFile << "expect \"*password:\" {set ifflg 1;send \"" << pwd << "\\r\"}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "\"*password:\" {set ifflg 1;send \"" << pwd << "\\r\"}" << endl;
    this->expectFile << "\"100%\" {set ifflg 0;puts \"==expect(:3)\"; send \"\\r\"}" << endl;
    this->expectFile << "\"*]$ \" {puts \"==expect(:-3)\"; exit}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "if { $ifflg == 1 } { " << endl;
    this->expectFile << "\texpect {" << endl;
    this->expectFile << "\t\"100%\" {puts \"==expect(:3)\"; send \"\\r\"}" << endl; //能看到100% 说明scp成功  否则失败
    this->expectFile << "\t\"*]$ \" {puts \"==expect(:-3)\"; exit}" << endl;
    this->expectFile << "\t}" << endl;
    this->expectFile << "} " << endl;
    this->expectFile << "expect eof" << endl;
    return 0;
}

int CCompiler::modulesMakeShell(ST_PROGRAM &stProgram) {
    string user=stProgram.stCompileInfo.stCompileHost.hostUser;
    string ip=stProgram.stCompileInfo.stCompileHost.hostIP;
    string pwd=stProgram.stCompileInfo.stCompileHost.hostPwd;
    int modnum=stProgram.v_stModule.size();
    this->expectFile << "spawn /usr/bin/ssh " << user << "@" << ip << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"*password:\" {send \"" << pwd << "\\r\"}" << endl;
    this->expectFile << "\"*]$ \" {send \"\\r\"}" << endl;
    this->expectFile << "}" << endl;
    ofstream ofs;
    ofs.open(this->execShell2.c_str());
    for(int i=0;i<modnum;i++){
        string execPath=stProgram.v_stModule[i].compileExecPath;
        string execCmd=stProgram.v_stModule[i].compileCmd;
        ofs << "cd " << execPath << " && (" << execCmd << "); ";
        ofs << "if [ $? -ne 0 ]; then echo -n \"F\";echo -n \"==make(:2)\"; fi";
        if(i == modnum-1){
            ofs << "\r";
        }else{
            ofs << "; ";
        }
    }
    ofs.flush();
    ofs.close();
    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "send [read [open " << this->execShell2 << " r] ]" << endl;
    this->expectFile << "expect {" << endl;
    this->expectFile << "\"F==make(:2)*]$ \" {puts \"==expect(:-4)\";exit}" << endl;
    this->expectFile << "\"*]$ \" {puts \"==expect(:4)\";send \"\\r\"}" << endl;
    this->expectFile << "}" << endl;
    this->expectFile << "expect \"*]$ \"" << endl;
    this->expectFile << "send \"logout\\r\"" << endl;
    this->expectFile << "expect eof" << endl;
    return 0;
}

int CCompiler::exec() {
    FILE* fp = NULL;
    string cmd = "expect "+this->expectUrl;
    char buf[1024]={0};
    if((fp=popen(cmd.c_str(),"r"))!=NULL){
        while(fgets(buf,sizeof(buf),fp)!=NULL){
            cout << buf;   //输出到控制台
            string sbuf = (string)buf;
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

int CCompiler::setFlowCode(const char *chcode) {
    int code = atoi(chcode);
    this->FlowArr[code]=1;
    return 0;
}

int CCompiler::setErrorCode(const char *chcode) {
    int code = atoi(chcode);
    this->ErrorArr[code]=1;
    return 0;
}

int CCompiler::analyzeRst() {
    if(this->FlowArr[0] == 1){  //全流程脚本执行到结尾
        int sumE=0;
        for(int i=1;i<5;i++){
            sumE+=this->ErrorArr[i];
        }
        if(sumE == 0){
            cout << "编译全流程成功" << endl;
            this->stRst.stage="SUCCESS✔";
            this->stRst.reason="...";
            return 0;
        }
    }

    this->stRst.stage="FAIL✘";
    int fi=0;
    for(int i=5;i>1;i--){
        if(this->ErrorArr[i-1]==1){
            fi=i;
            break;
        }
    }
    switch(fi){
        case 1:
            this->stRst.reason="ssh登陆编译主机失败";
            break;
        case 2:
            this->stRst.reason="检查编译主机源码路径-源码备份-历史源码清理失败";
            break;
        case 3:
            this->stRst.reason="scp复制源码文件夹到编译主机失败";
            break;
        case 4:
            this->stRst.reason="编译失败";
            break;
        default:
            this->stRst.reason="unknown";
    }
    return -1;  //出错
}

ST_RST CCompiler::getRst() {
    return this->stRst;
}

#endif

