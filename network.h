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
#include <QList>
#include <QHash>

struct boardInfo{
    QString serialNum;
    quint16 length;
    quint16 width;
    quint32 total;
    quint32 ngcount;
    quint32 okcount;
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
    private:
        void init();

    private Q_SLOTS:
        void readMCUData();
        void getPhoneInfo();
        void establishNewConnection();
        void errorOccur(QAbstractSocket::SocketError);
        void transferStateChanged(QAbstractSocket::SocketState);
        void networkError(QAbstractSocket::SocketError);
        void deleteTransferSocket();
        void tcpStateChanged(QAbstractSocket::SocketState);
    
};

inline QDataStream &operator<<(QDataStream &out, const struct boardInfo &board)
{
    out.setVersion(QDataStream::Qt_5_0);
    out<<board.serialNum<< board.length<< board.width<<
        board.total<< board.ngcount<< board.okcount;
    return out;
}

#endif
