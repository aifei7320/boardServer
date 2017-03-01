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
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QSqlQuery>
#include <QDataStream>
#include <QMetaType>
#include <QTcpSocket>
#include <QSemaphore>
#include <QQueue>
#include <QVector>
#include <QList>
#include <QTimer>
#include <QHash>

struct boardInfo{
    qint32 magicNum = -1;
    QString serialNum;
    qint16 length = -1;
    qint16 width = -1;
    qint32 total = -1;
    qint32 ngcount = -1;
    qint32 okcount = -1;
    qint8 lengthMatch = -1;
    qint8 widthMatch = -1;
    qint8 boardPerfect = -1;
    qint16 devNum = -1;
};
Q_DECLARE_METATYPE(boardInfo)

class Network : public QObject
{
    Q_OBJECT
    public:
        explicit Network(QObject *parent=0);

    private:
        QList<QTcpSocket *> mcuTcpSocketList;
        QTcpServer *phoneServer;
        QHash<qint32, QTcpSocket*> socketHashTable;

        QTcpSocket *transferSocket;
        QTimer *timer;
    private:
        void init();

    private Q_SLOTS:
        void readData();
        void getPhoneInfo();
        void establishNewConnection();
        void errorOccur(QAbstractSocket::SocketError);
        void transferStateChanged(QAbstractSocket::SocketState);
        void networkError(QAbstractSocket::SocketError);
        void deleteTransferSocket();
        void tcpStateChanged(QAbstractSocket::SocketState);
        void testConnection();
        void reConn();
    
};

inline QDataStream &operator<<(QDataStream &out, const struct boardInfo &board)
{
    out.setVersion(QDataStream::Qt_5_0);
    out<< 123456<< board.serialNum<< board.length<< board.width<<
        board.total<< board.ngcount<< board.okcount<< board.lengthMatch<<
        board.widthMatch<< board.boardPerfect;
    return out;
}

inline QDataStream &operator<<(QDataStream &out, const struct boardInfo *&board)
{
    out.setVersion(QDataStream::Qt_5_0);
    out<< 123456<< board->serialNum<< board->length<< board->width<<
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
