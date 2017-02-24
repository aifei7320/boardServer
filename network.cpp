/*************************************************************************
    > File Name: network.cpp
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年02月21日 星期二 15时15分45秒
 ************************************************************************/

#include <iostream>
#include <unistd.h>
using namespace std;

#include "network.h"

QSemaphore readSema(0);
QSemaphore writeSema(100);
QQueue<QByteArray> resource;

Network::Network(QObject *parent) : QObject(parent),
    phoneServer(new QTcpServer)
{
    QString str("192.168.0.168:8000@1234");
    qDebug()<<str.indexOf(":")<<str.left(str.indexOf(":"))<<str.mid(str.indexOf(":") + 1, str.indexOf("@") - str.indexOf(":") - 1);
    init();
}

void Network::init()
{
    phoneServer->listen(QHostAddress::Any, 7320);
    connect(phoneServer, SIGNAL(newConnection()), this, SLOT(establishNewConnection()));

    for (int i = 0; i < 16; ++i){
        mcuTcpSocketList.append(new QTcpSocket);
        mcuTcpSocketList[i]->setObjectName(QString::number(i));
        mcuTcpSocketList[i]->connectToHost("192.168.0.32", 4000 + i);
        connect(mcuTcpSocketList[i], SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(networkError(QAbstractSocket::SocketError)));
        connect(mcuTcpSocketList[i], SIGNAL(readyRead()), this, SLOT(readMCUData()));

        qDebug()<<"create socket";
    }
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

void Network::readMCUData()
{
    struct boardInfo m;
    m.width = 650;
    m.length = 952;
    m.serialNum = "";
    m.total = 1000;
    m.ngcount = 5;
    m.okcount = 995;
    m.lengthMatch = 0;
    m.widthMatch = 0;
    m.boardPerfect = 0;
    qDebug()<<m.serialNum;

    QTcpSocket *tcp=NULL;
    quint8 header;
    QByteArray info;
    quint8 dev;
    tcp = static_cast<QTcpSocket*>(sender());
    QDataStream in(tcp);
    in >> header;
    if (header != 'M'){
        tcp->readAll();
        return;
    }
    while(tcp->bytesAvailable()<25);
    info = tcp->read(25);
    qDebug()<< info<<tcp->objectName();
    dev = tcp->objectName().toInt();
    writeSema.release();
    if(socketHashTable[dev] != NULL){
        socketHashTable[dev]->write(info);
        //QDataStream out(socketHashTable[dev]);
        //out<< m;
        qDebug()<< info<<tcp->objectName();
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
