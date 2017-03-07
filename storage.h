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
#include <QSqlError>

#include <iostream>
#include <unistd.h>

struct boardInfo{
    qint32 magicNum = -1;
    QString serialNum;
    qint16 length = -1;
    qint16 width = -1;
    qreal realWidth = 0.0;
    qreal realLength = 0.0;
    qint32 total = -1;
    qint32 ngcount = -1;
    qint32 okcount = -1;
    qint8 lengthMatch = -1;
    qint8 widthMatch = -1;
    qint8 boardPerfect = -1;
    qint16 devNum = -1;
};
Q_DECLARE_METATYPE(boardInfo)

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
private Q_SLOTS:
    void deletethis();
};


#endif
