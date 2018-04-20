//
// Created by BaiFF on 2018/4/12.
//

#include "loader.h"
#include "rmsAes.h"
#include "comm.h"

#ifdef CJENKINS_LOADER_H

ST_PROGRAM CLoader::getStProgram() {
    return this->st_task.st_program;
}

ST_GROUP CLoader::getStGroup() {
    return  this->st_task.st_group;
}

CLoader::CLoader(string task,string taskType,string taskEnv) {
    this->init(task,taskType,taskEnv);
}

int CLoader::init(string task,string taskType,string taskEnv) {
    string strConn="Driver={MySQL ODBC 5.1 Driver};"+CConf::get_cfg_dbInfo();
    const char* connStr = strConn.c_str();

    try{
        otl_connect::otl_initialize();
        this->dbConn.rlogon(connStr);
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }

    otl_stream osm;
    if("program" == taskType){
        this->loadStProgram(task,taskEnv,osm);
    }else if("group" == taskType){
        this->loadStGroup(task,taskEnv,osm);
    }else{
        return -1;
    }
    return 0;
}

int CLoader::loadStProgram(string task, string taskEnv, otl_stream &osm) {
    this->st_task.st_program.programName=task;
    this->loadStCompile(task,taskEnv,osm);
    this->loadStDeploy(task,taskEnv,osm);
    this->loadStModule(task,taskEnv,osm);
    return 0;
}

int CLoader::loadStGroup(string task, string taskEnv, otl_stream &osm) {
    this->st_task.st_group.groupName=task;
    this->st_task.st_group.v_stProgram.clear();
    char sql_buf[1024]={0};
    char tmp_buf[100]={0};
    vector<string> vProgram;
    sprintf(sql_buf,
            "select program_name from i_app_program_group_%s "
                    "where program_group='%s' and program_status=1;",
            taskEnv.c_str(),task.c_str());
    try{
        osm.open(1024,sql_buf,this->dbConn);
        while(!osm.eof()){
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            vProgram.push_back((string)tmp_buf);
        }
        osm.close();
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }
    for(int i=0;i<vProgram.size();i++){
        this->loadStProgram(vProgram[i],taskEnv,osm);
        this->st_task.st_group.v_stProgram.push_back(this->st_task.st_program);
    }
    return 0;
}

int CLoader::loadStCompile(string task, string taskEnv, otl_stream &osm) {
    char sql_buf[1024]={0};
    char tmp_buf[2048]={0};
    sprintf(sql_buf,
            "select a.compile_host_ip,a.compile_host_user,b.host_pwd,a.compile_src_path "
                    "from i_app_compile_info_base_%s a,i_app_host_info_%s b "
                    "where a.program_name='%s' "
                    "and a.compile_status=1 "
                    "and b.host_ip=a.compile_host_ip "
                    "and b.host_user=a.compile_host_user "
                    "and b.host_status=1;",
            taskEnv.c_str(),taskEnv.c_str(),task.c_str());
    try{
        osm.open(1024,sql_buf,this->dbConn);
        if(!osm.eof()){
            osm >> tmp_buf;
            this->st_task.st_program.stCompileInfo.stCompileHost.hostIP=(string)tmp_buf;
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            this->st_task.st_program.stCompileInfo.stCompileHost.hostUser=(string)tmp_buf;
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            int oriPwdLen=0;
            for(int i=0;i< sizeof(tmp_buf);i++){
                if(tmp_buf[i] != '\0'){
                    oriPwdLen++;
                }else{
                    break;
                }
            }
            char* outPwd=rmsAesDecode(tmp_buf, oriPwdLen);
            this->st_task.st_program.stCompileInfo.stCompileHost.hostPwd=(string)outPwd;
            if(outPwd != NULL){
                rmsAesFree(&outPwd);
            }
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            this->st_task.st_program.stCompileInfo.compileSrcPath=(string)tmp_buf;
        }
        osm.close();
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }
    return 0;
}

int CLoader::loadStDeploy(string task, string taskEnv, otl_stream &osm) {
    char sql_buf[1024]={0};
    char tmp_buf[2048]={0};
    int tmp_int=0;
    sprintf(sql_buf,
            "select a.deploy_host_ip,a.deploy_host_user,b.host_pwd "
                    "from i_app_deploy_info_base_%s a,i_app_host_info_%s b "
                    "where a.program_name='%s' "
                    "and a.deploy_status=1 "
                    "and b.host_ip=a.deploy_host_ip "
                    "and b.host_user=a.deploy_host_user "
                    "and b.host_status=1;",
            taskEnv.c_str(),taskEnv.c_str(),task.c_str());
    try{
        osm.open(1024,sql_buf,this->dbConn);
        while(!osm.eof()){
            ST_HOST st_host_tmp;
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            st_host_tmp.hostIP=(string)tmp_buf;
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            st_host_tmp.hostUser=(string)tmp_buf;
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            int oriPwdLen=0;
            for(int i=0;i< sizeof(tmp_buf);i++){
                if(tmp_buf[i] != '\0'){
                    oriPwdLen++;
                }else{
                    break;
                }
            }
            char* outPwd=rmsAesDecode(tmp_buf, oriPwdLen);
            st_host_tmp.hostPwd=(string)outPwd;
            if(outPwd != NULL){
                rmsAesFree(&outPwd);
            }
            this->st_task.st_program.stDeployInfo.v_stDeployHost.push_back(st_host_tmp);
        }
        osm.close();
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }

    memset(sql_buf,0x00,sizeof(sql_buf));
    sprintf(sql_buf,
            "select pre_shell,after_shell,if_deploy_srccode "
                    "from i_app_deploy_info_extra_%s "
                    "where program_name='%s' and deploy_status=1;",
            taskEnv.c_str(),task.c_str());
    try{
        osm.open(1024,sql_buf,this->dbConn);
        if(!osm.eof()){
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            this->st_task.st_program.stDeployInfo.preShell=(string)tmp_buf;
            memset(tmp_buf,0x00, sizeof(tmp_buf));
            osm >> tmp_buf;
            this->st_task.st_program.stDeployInfo.afterShell=(string)tmp_buf;
            osm >> tmp_int;
            this->st_task.st_program.stDeployInfo.ifDeploySrc=tmp_int;
        }
        osm.close();
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }

    return 0;
}

int CLoader::loadStModule(string task, string taskEnv, otl_stream &osm) {
    char sql_buf[1024]={0};
    char tmp_buf[2048]={0};
    sprintf(sql_buf,
            "select module_name,compile_exec_path,complie_cmd,deploy_url,deploy_path "
                    "from i_app_module_info_%s "
                    "where program_name='%s'and moudle_status=1;",
            taskEnv.c_str(),task.c_str());
    try{
        osm.open(1024,sql_buf,this->dbConn);
        while(!osm.eof()){
            ST_MODULE st_module_tmp;
            memset(tmp_buf,0x00,sizeof(tmp_buf));
            osm >> tmp_buf;
            st_module_tmp.modName=(string)tmp_buf;
            memset(tmp_buf,0x00,sizeof(tmp_buf));
            osm >> tmp_buf;
            st_module_tmp.compileExecPath=(string)tmp_buf;
            memset(tmp_buf,0x00,sizeof(tmp_buf));
            osm >> tmp_buf;
            st_module_tmp.compileCmd=(string)tmp_buf;
            memset(tmp_buf,0x00,sizeof(tmp_buf));
            osm >> tmp_buf;
            st_module_tmp.deployUrl=(string)tmp_buf;
            memset(tmp_buf,0x00,sizeof(tmp_buf));
            osm >> tmp_buf;
            st_module_tmp.deployPath=(string)tmp_buf;
            this->st_task.st_program.v_stModule.push_back(st_module_tmp);
        }
        osm.close();
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }

    return 0;
}

CLoader::~CLoader() {
    try{
        this->dbConn.logoff();
    }catch(otl_exception &e){
        cout << e.msg << endl;
        cout << e.stm_text << endl;
        cout << e.var_info << endl;
    }
}

#endif
