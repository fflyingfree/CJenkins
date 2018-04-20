//
// Created by BaiFF on 2018/4/12.
//

#ifndef CJENKINS_LOADER_H
#define CJENKINS_LOADER_H

#include "comm.h"
#include "conf.h"

#define OTL_ODBC_UNIX
#include "otlv4.h"

class CLoader{
public:
    ST_PROGRAM getStProgram();
    ST_GROUP getStGroup();
    //taskType:program group  //taskEnv:test online
    CLoader(string task,string taskType,string taskEnv);
    ~CLoader();
private:
    //taskType:program group  //taskEnv:test online
    int init(string task,string taskType,string taskEnv);
    int loadStProgram(string task,string taskEnv,otl_stream &osm);
    int loadStCompile(string task,string taskEnv,otl_stream &osm);
    int loadStDeploy(string task,string taskEnv,otl_stream &osm);
    int loadStModule(string task,string taskEnv,otl_stream &osm);
    int loadStGroup(string task,string taskEnv,otl_stream &osm);
private:
    struct{
        ST_PROGRAM st_program;
        ST_GROUP st_group;
    } st_task;
    otl_connect dbConn;
};

#endif //CJENKINS_LOADER_H
