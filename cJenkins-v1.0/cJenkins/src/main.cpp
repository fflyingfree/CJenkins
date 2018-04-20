//后台持续集成--核心服务程序
//create by baiff3 at 2018/4/18
//version 1.0
#include "loader.h"
#include "compiler.h"
#include "deployer.h"
#include "recorder.h"
#include "comm.h"

int pProcess(ST_PROGRAM &stp);
int showResult(ST_RST &stRst_c,vector<ST_RST> &v_stRst_d);     //最终结果展示
int recordInfo(ST_RST &stRst_c,vector<ST_RST> &v_stRst_d);

//主程序
//以exit($退出码)的形式结束，以便shell根据状态码判断是否执行成功
//0 success ， 2 fail  （粗粒度判断）
int main(int argc,char* argv[]) {
    int ret = 0;  //程序退出码  初始假设为0（正常退出）

    //参数处理
    //参数1：程序名或组名  参数2：类型是程序还是组   参数3：测试还是生产环境
    if(argc != 4
            || ((string)argv[2] != "program" && (string)argv[2] != "group")
            || ((string)argv[3] != "test" && (string)argv[3] != "online")){
        cout << "【cJenkins】参数错误！" << endl;
        cout << "参数1：程序名或组名,\"程序名-jenkins任务名-编译发布源码文件夹名\"需要保持统一" << endl;
        cout << "参数2：参数1为程序名则参数2为program，参数1为组名则参数2为group，必须小写" << endl;
        cout << "参数3：测试环境则为test，生产环境则为online，必须小写" << endl;
        cout << "参数之间空格分隔" << endl;
        exit(2);
    }

    string task = (string)argv[1];
    string type = (string)argv[2];
    string env = (string)argv[3];

    CLoader* cl = new CLoader(task,type,env);

    if(type == "program"){
        ST_PROGRAM stp = cl->getStProgram();
        cout << ">>>>>>>>>>>>>>>>>>>> program: " << stp.programName << " >>>>>>>>>>>>>>>>>>>>" << endl;
        ret = pProcess(stp);
    }else if(type == "group"){
        ST_GROUP stg = cl->getStGroup();
        cout << ">>>>>>>>>>>>>>>>>>>> group: " << stg.groupName << " >>>>>>>>>>>>>>>>>>>>\r\n" << endl;
        for(int i=0;i<stg.v_stProgram.size();i++){
            int rst = 0;
            cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> g-program: " << stg.v_stProgram[i].programName << " >>>>>>>>>>>>>>>>>>>>" << endl;
            rst = pProcess(stg.v_stProgram[i]);
            if(rst != 0){
                ret = rst;
            }
        }
    }else{
        exit(2);
    }
    delete(cl);

    std::cout << "Hello, World!" << std::endl;
    cout << "main exit with ret:[" << ret << "]" << endl;
    exit(ret);
}

int pProcess(ST_PROGRAM &stp){
    int ret=0;

    int rst=0;
    CCompiler cpl;
    rst=cpl.process(stp);
    if(rst !=0){
        ret=rst;
    }
    ST_RST stRst_c = cpl.getRst();

    vector<ST_RST> v_stRst_d;
    v_stRst_d.clear();
    if(ret == 0){  //编译成功
        rst=CMulDeployer::deploy(stp);
        if(rst != 0){   //结果不等于0标识执行异常
            ret=rst;
        }
        v_stRst_d = CMulDeployer::getVrsts();
    }

    showResult(stRst_c,v_stRst_d);
    recordInfo(stRst_c,v_stRst_d);

    return ret;
}

int showResult(ST_RST &stRst_c,vector<ST_RST> &v_stRst_d){
    cout << endl;
    cout << "*** 编译结果 **************************************************************" << endl;
    cout << "* 【程序名】 / 【编译主机】 / 【编译结果】 / 【原因】 *" << endl;
    cout << "* " << stRst_c.programName <<" / " << stRst_c.hostIP << " / " << stRst_c.stage << " / " << stRst_c.reason << " *" << endl;
    cout << "**************************************************************************" << endl;
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "*** 发布结果 **************************************************************" << endl;
    cout << "* 【程序名】 / 【发布主机】 / 【发布结果】 / 【原因】 *" << endl;
    for(int i=0;i<v_stRst_d.size();++i){
        cout << "* " << v_stRst_d[i].programName <<" / " << v_stRst_d[i].hostIP << " / " << v_stRst_d[i].stage << " / " << v_stRst_d[i].reason << " *" << endl;
    }
    cout << "**************************************************************************" << endl;
    cout << endl;
    return 0;
}

int recordInfo(ST_RST &stRst_c,vector<ST_RST> &v_stRst_d){
    vector<ST_RECORD> vRecords;
    vRecords.clear();

    for(int i=0;i<v_stRst_d.size();++i){
        ST_RECORD sr;
        sr.programName=stRst_c.programName;
        sr.cHost=stRst_c.hostIP;
        sr.dHost=v_stRst_d[i].hostIP;
        sr.cRst=(stRst_c.stage+" -- "+stRst_c.reason);
        sr.dRst=(v_stRst_d[i].stage+" -- "+v_stRst_d[i].reason);
        if(stRst_c.stage=="SUCCESS✔" && v_stRst_d[i].stage=="SUCCESS✔"){
            sr.rst="0";
            sr.rstDesc="SUCCESS✔";
        }else{
            sr.rst="2";
            sr.rstDesc="FAIL✘";
        }

        vRecords.push_back(sr);
    }

    if(vRecords.size() > 0){
        CRecorder* cr = new CRecorder();
        cr->record(vRecords);   //结果记录入mysql
        delete(cr);
    }

    return 0;
}

