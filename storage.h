/*************************************************************************
    > File Name: storage.h
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年02月25日 星期六 11时53分05秒
 ************************************************************************/
#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <QThread>
#include <QString>
#include <QByteArray>
#include <QSemaphore>
#include <QVector>
#include <QSqlQuery>
#include <QDebug>
#include "network.h"
#include <QSqlError>

#include <iostream>
#include <unistd.h>

typedef struct boardInfo page;
using namespace std;

class Storage : public QThread
{
    Q_OBJECT
public:

    Storage(QObject *parent=0);
    void stop(){stopRunning = false;}

protected:

    void run()Q_DECL_OVERRIDE;
private:

    bool stopRunning;
};


#endif
