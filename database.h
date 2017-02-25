/*************************************************************************
  > File Name: database.h
  > Author: zxf
  > Mail: zhengxiaofeng333@163.com 
  > Created Time: 2017年02月25日 星期六 09时57分45秒
 ************************************************************************/
#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <iostream>
#include <unistd.h>
using namespace std;

#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>

QSqlDatabase mySqlDb;
static bool connectToDatabase()
{
    mySqlDb = QSqlDatabase::addDatabase("QMYSQL");
    mySqlDb.setHostName("localhost");
    mySqlDb.setDatabaseName("boardInfo");
    mySqlDb.setUserName("root");
    mySqlDb.setPassword("shelly");
    mySqlDb.setPort(3306);
    bool ok = mySqlDb.open();
    if (!ok){
        QMessageBox::warning(0, QObject::tr("Error"), mySqlDb.lastError().text());
        return false;
    }

    return true;
}

#endif
