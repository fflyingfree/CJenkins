//
// Created by BaiFF on 2018/4/19.
//

#ifndef CJENKINS_RECORDER_H
#define CJENKINS_RECORDER_H

#include "comm.h"
#include "conf.h"

#define OTL_ODBC_UNIX
#define OTL_BIGINT long long
#include "otlv4.h"

class CRecorder{
public:
    int record(vector<ST_RECORD> &vRecord);
    CRecorder();
    ~CRecorder(){ };
private:
    int init();
    int makeInSql(vector<ST_RECORD> &vRecord,string &sql);
    OTL_BIGINT getTaskID();
private:
    otl_connect dbConn;
    string env;
};

#endif //CJENKINS_RECORDER_H
