/*************************************************************************
    > File Name: /home/hero/project/boardServer/storage.cpp
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年02月25日 星期六 12时02分58秒
 ************************************************************************/

#include <iostream>
#include <unistd.h>
#include "storage.h"
using namespace std;

extern QSemaphore readSema;
extern QSemaphore writeSema;
extern QVector<struct boardInfo*> resource;
//extern QSqlDatabase mySqlDb;

Storage::Storage(QObject *parent) : QThread(parent),
    stopRunning(true)
{
}

void Storage::run()
{
    QSqlQuery query;
    while(stopRunning){
        while(readSema.available() > 0){

            readSema.acquire();
            page *storage = resource.takeFirst();
            writeSema.release();
            query.prepare("insert into boardInfo( serialNum, devNum, width, length, realWidth, realLength, boardPerfect) values"
                    "(:serialNum, :devNum, :width, :length,:realWidth, :realLength, :boardPerfect)");
            query.bindValue(":serialNum", storage->serialNum);
            query.bindValue(":devNum", storage->devNum);
            query.bindValue(":width", storage->width);
            query.bindValue(":length", storage->length);
            query.bindValue(":realWidth", storage->realWidth);
            query.bindValue(":realLength", storage->realLength);
            query.bindValue(":boardPerfect", storage->boardPerfect);
            qDebug()<< query.lastError();
            query.exec();
            if (storage != NULL)
                delete storage;
            }
    }
}

void Storage::deletethis()
{
    qDebug()<<"delete";
    this->stop();
}
