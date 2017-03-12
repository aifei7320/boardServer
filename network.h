/*************************************************************************
  > File Name: network.h
  > Author: zxf
  > Mail: zhengxiaofeng333@163.com 
  > Created Time: 2017年02月21日 星期二 13时32分24秒
 ************************************************************************/

#ifndef _NETWORK_H_
#define _NETWORK_H_



#include <iostream>
#include <unistd.h>
using namespace std;


#include <QTcpServer>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QSqlQuery>
#include <QDataStream>
#include <QMetaType>
#include <QTcpSocket>
#include <QSqlError>
#include <QSemaphore>
#include <QQueue>
#include <QVector>
#include <QMessageBox>
#include <QList>
#include <QTimer>
#include <QSqlQuery>
#include <QHash>
#include "storage.h"

class Network : public QObject
{
    Q_OBJECT
    public:
        explicit Network(QObject *parent=0);
        ~Network();

    private:
        QHash<qint32, QTcpSocket *> mcuTcpSocketList;
        QTcpServer *phoneServer;
        QHash<qint32, QTcpSocket*> socketHashTable;

        QTcpSocket *transferSocket;
        QTimer *timer;
        Storage *storage;
    private:
        void init();

        private Q_SLOTS:
        void readData();
        void readDoubleData();
        void getPhoneInfo();
        void establishNewConnection();
        void errorOccur(QAbstractSocket::SocketError);
        void transferStateChanged(QAbstractSocket::SocketState);
        void networkError(QAbstractSocket::SocketError);
        void deleteTransferSocket();
        void tcpStateChanged(QAbstractSocket::SocketState);
        void connectionStateChanged(QAbstractSocket::SocketState);
        void reConn();

};

inline QDataStream &operator<<(QDataStream &out, const struct boardInfo &board)
{
    out.setVersion(QDataStream::Qt_5_0);
    out<< 123456<< board.serialNum<< board.length<< board.width<<
        board.realWidth<< board.realLength << board.total<< board.ngcount<<
        board.okcount<< board.lengthMatch<< board.widthMatch<< board.boardPerfect;
    return out;
}

inline QDataStream &operator<<(QDataStream &out, const struct boardInfo *&board)
{
    out.setVersion(QDataStream::Qt_5_0);
    out<< 123456<< board->serialNum<< board->length<< board->width<< board->realWidth<< board->realLength<<
        board->total<< board->ngcount<< board->okcount<< board->lengthMatch<<
        board->widthMatch<< board->boardPerfect;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, struct boardInfo &board)
{
    in.setVersion(QDataStream::Qt_5_0);
    in>> board.magicNum;
    if (board.magicNum != 123456){
        board.magicNum = 0;
        return in;
    }
    in>>board.serialNum>> board.length>> board.width>>
        board.total>> board.ngcount>> board.okcount>> board.lengthMatch>>
        board.widthMatch>> board.boardPerfect;
    return in;
}

inline QDataStream &operator>>(QDataStream &in, struct boardInfo *&board)
{
    in.setVersion(QDataStream::Qt_5_0);
    in>> board->magicNum;
    if (board->magicNum != 123456){
        board->magicNum = 0;
        return in;
    }
    in>>board->serialNum>> board->length>> board->width>>
        board->total>> board->ngcount>> board->okcount>> board->lengthMatch>>
        board->widthMatch>> board->boardPerfect;
    return in;
}
#endif
