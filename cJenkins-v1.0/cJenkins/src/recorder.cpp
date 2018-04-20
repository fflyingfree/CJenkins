//
// Created by BaiFF on 2018/4/19.
//

#include "recorder.h"
#include "comm.h"

#ifdef  CJENKINS_RECORDER_H

int CRecorder::record(vector<ST_RECORD> &vRecord) {
    string sql="";
    this->makeInSql(vRecord,sql);
    try{
        otl_stream osm;
        osm.open(1,sql.c_str(),this->dbConn);
        osm.flush();
        osm.close();
        this->dbConn.commit();
    }catch(otl_exception &e){
        cerr << e.msg << endl;
        cerr << e.stm_text << endl;
        cerr << e.var_info << endl;
    }

    try{
        this->dbConn.logoff();
    }catch(otl_exception &e){
        cerr << e.msg << endl;
        cerr << e.stm_text << endl;
        cerr << e.var_info << endl;
    }

    return 0;
}

int CRecorder::makeInSql(vector<ST_RECORD> &vRecord, string &sql) {
    OTL_BIGINT taskID=this->getTaskID();
    char buf[21]={0};
    sprintf(buf,"%lld",taskID);
    string strTaskID=(string)buf;
    sql+="insert into i_app_integrat_log_";
    sql+=this->env;
    sql+=" (task_id,program_name,compile_host,deploy_host,c_rst,d_rst,i_rst,rst_desc) values ";
    for(int i=0;i<vRecord.size();++i){
        sql+="(";
        sql+=(strTaskID+",");
        sql+=("'"+vRecord[i].programName+"',");
        sql+=("'"+vRecord[i].cHost+"',");
        sql+=("'"+vRecord[i].dHost+"',");
        sql+=("'"+vRecord[i].cRst+"',");
        sql+=("'"+vRecord[i].dRst+"',");
        sql+=(vRecord[i].rst+",");
        sql+=("'"+vRecord[i].rstDesc+"'");
        sql+=")";
        if(i != vRecord.size()-1){
            sql+=",";
        }
    }
    sql+=";";
    return 0;
}

OTL_BIGINT CRecorder::getTaskID() {
    OTL_BIGINT taskID=0;
    string sql="";
    sql+="select max(task_id) from i_app_integrat_log_";
    sql+=this->env;
    sql+=";";
    try{
        otl_stream osm;
        osm.open(1024,sql.c_str(),this->dbConn);
        if(!osm.eof()){
            osm >> taskID;
        }
        osm.close();
        this->dbConn.commit();
    }catch(otl_exception &e){
        cerr << e.msg << endl;
        cerr << e.stm_text << endl;
        cerr << e.var_info << endl;
        taskID=-1;
    }

    return taskID+1;
}

int CRecorder::init() {
    string strConn="Driver={MySQL ODBC 5.1 Driver};"+CConf::get_cfg_dbInfo();
    const char* connStr = strConn.c_str();

    try{
        otl_connect::otl_initialize();
        this->dbConn.rlogon(connStr);
    }catch(otl_exception &e){
        cerr << e.msg << endl;
        cerr << e.stm_text << endl;
        cerr << e.var_info << endl;
    }

    this->env=CConf::get_cfg_env();
    return 0;
}

CRecorder::CRecorder() {
    this->init();
}

#endif

