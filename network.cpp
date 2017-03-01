/*************************************************************************
    > File Name: network.cpp
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年02月21日 星期二 15时15分45秒
 ************************************************************************/

#include <iostream>
#include <unistd.h>
#include <string.h>
using namespace std;

#include "network.h"

QSemaphore readSema(0);
QSemaphore writeSema(100);
QVector<struct boardInfo*> resource;

extern QSqlDatabase mySqlDb;

Network::Network(QObject *parent) : QObject(parent),
    phoneServer(new QTcpServer)
{
    storage = new Storage;
    storage->start();
    init();
    timer = new QTimer;
}

Network::~Network()
{
    mcuTcpSocketList.clear();
    socketHashTable.clear();
    storage->stop();
    storage->deleteLater();
    delete timer;
}

void Network::init()
{
    phoneServer->listen(QHostAddress::Any, 7320);
    connect(phoneServer, SIGNAL(newConnection()), this, SLOT(establishNewConnection()));

    QSqlQuery query(mySqlDb);
    query.exec("select ip, port, devNum from deviceList");
#if 1
    int count = 0;
    while(query.next()){
        mcuTcpSocketList.append(new QTcpSocket);
        mcuTcpSocketList[count]->setObjectName(query.value(2).toString());
        mcuTcpSocketList[count]->connectToHost(query.value(0).toString(), query.value(1).toInt());
        connect(mcuTcpSocketList[count], SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(networkError(QAbstractSocket::SocketError)));
        connect(mcuTcpSocketList[count], SIGNAL(readyRead()), this, SLOT(readData()));
        qDebug()<<"create socket";
        count++;
    }
#else
    for (int i = 0; i < 16; ++i){
        mcuTcpSocketList.append(new QTcpSocket);
        mcuTcpSocketList[i]->setObjectName(QString::number(i));
        mcuTcpSocketList[i]->connectToHost("192.168.0.32", 4000 + i);
        connect(mcuTcpSocketList[i], SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(networkError(QAbstractSocket::SocketError)));
        connect(mcuTcpSocketList[i], SIGNAL(readyRead()), this, SLOT(readData()));

        qDebug()<<"create socket";
    }
#endif
}

void Network::establishNewConnection()
{

    disconnect(phoneServer, SIGNAL(newConnection()), this, SLOT(establishNewConnection()));
    transferSocket = phoneServer->nextPendingConnection();
    connect(transferSocket, SIGNAL(readyRead()), this, SLOT(getPhoneInfo()));
    connect(transferSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(transferStateChanged(QAbstractSocket::SocketState)));
    connect(transferSocket, SIGNAL(disconnected()), this, SLOT(deleteTransferSocket()));

}

void Network::getPhoneInfo()
{
    QByteArray phoneInfo;
    QString ip;
    qint32 port;
    qint32 dev;
    quint8 infoLength;

    QDataStream in(transferSocket);
    in >> infoLength;

    while(transferSocket->bytesAvailable() < infoLength);
    phoneInfo = transferSocket->read(infoLength);

    ip = transferSocket->peerAddress().toString();
    ip = ip.right(ip.size() - ip.lastIndexOf(":") - 1);
    port = phoneInfo.left(phoneInfo.indexOf("@")).toInt();
    dev = phoneInfo.right(phoneInfo.size() - phoneInfo.indexOf("@") - 1).toInt();
    qDebug()<<ip<<port<<dev;
    qDebug()<<"peer ip"<<transferSocket->peerAddress().toString();
    QTcpSocket *tcp = new QTcpSocket;
    tcp->setObjectName(QString::number(dev));
    tcp->connectToHost(ip, port);
    connect(tcp, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(tcpStateChanged(QAbstractSocket::SocketState)));
    transferSocket->disconnectFromHost();
    socketHashTable.insert(dev, tcp);
}

void Network::transferStateChanged(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::UnconnectedState){
        qDebug()<<"unconnected";
    }

}

void Network::reConn()
{
    for (int i = 0; i < 16; ++i){
        mcuTcpSocketList[i]->connectToHost("192.168.0.32", 4000 + i);
    }

}

void Network::deleteTransferSocket()
{
    QTcpSocket *tcp = static_cast<QTcpSocket*>(sender());
    tcp->deleteLater();
    disconnect(transferSocket, SIGNAL(readyRead()), this, SLOT(getPhoneInfo()));
    disconnect(transferSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(transferStateChanged(QAbstractSocket::SocketState)));
    disconnect(transferSocket, SIGNAL(disconnected()), this, SLOT(deleteTransferSocket()));
    connect(phoneServer, SIGNAL(newConnection()), this, SLOT(establishNewConnection()));
    qDebug()<<"deleted";
}

void Network::errorOccur(QAbstractSocket::SocketError err)
{
    QTcpSocket* tcp = NULL;
    tcp = static_cast<QTcpSocket*>(sender());
    qDebug()<<tcp->objectName()<<err;
}

void Network::readData()
{
    struct boardInfo *storage;
    QTcpSocket *tcp=NULL;
    quint8 header;
    QByteArray info;
    quint8 dev;
    tcp = static_cast<QTcpSocket*>(sender());
    QDataStream in(tcp);
    in >> header;
    if (header != 'H'){
        tcp->readAll();
        return;
    }
    while(tcp->bytesAvailable()<29);
    info = tcp->read(29);
    dev = tcp->objectName().toInt();
    storage = new struct boardInfo;
    qDebug()<<info;

    storage->serialNum = info.left(info.indexOf('s')); 
    storage->length = info.mid(info.lastIndexOf('s') + 1, info.indexOf('L') - info.lastIndexOf('s') - 1).toInt();
    storage->width = info.mid(info.lastIndexOf('L') + 1, info.indexOf('W') - info.lastIndexOf('L') - 1).toInt();
    storage->boardPerfect = info.at(28) - 0x30;
    storage->devNum = dev;

    writeSema.acquire();
    resource.append(storage);
    readSema.release();
    if(socketHashTable[dev] != NULL){
        //socketHashTable[dev]->write(info);
        QDataStream out(socketHashTable[dev]);
        out<<*storage;
        qDebug()<<storage->width<< storage->length<< storage->serialNum<< storage->total;
    }
}

void Network::networkError(QAbstractSocket::SocketError err)
{
    QTcpSocket *tcp;
    tcp = static_cast<QTcpSocket*>(sender());
    switch (tcp->objectName().toInt()){
        case 1:{
                qDebug()<<tcp->objectName()<<err;
                    
                    break;
                }
        case 2:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 3:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 4:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 5:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 6:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 7:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 8:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 9:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 10:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 11:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 12:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 13:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 14:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }
        case 15:{
                qDebug()<<tcp->objectName()<<err;

                    break;
                }

    }
}

void Network::testConnection()
{
}

void Network::tcpStateChanged(QAbstractSocket::SocketState s)
{
    quint8 dev;
    dev = static_cast<QTcpSocket*>(sender())->objectName().toInt();
    switch(s){
        case QAbstractSocket::ConnectedState:
            break;
        case QAbstractSocket::ClosingState:
            socketHashTable.remove(dev);
            break;
        case QAbstractSocket::ListeningState:
            break;
        case QAbstractSocket::UnconnectedState:
            break;
        case QAbstractSocket::ConnectingState:
            break;
        case QAbstractSocket::BoundState:
            break;
        default:break;

    }
    qDebug()<<s;
}
