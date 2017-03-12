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

QMutex mutex;
QWaitCondition waitCondition;

//extern QSqlDatabase mySqlDb;

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

    QSqlQuery query;
    qint32 devNum;
    query.exec("select ip, port, devNum from deviceList");
#if 1
    while(query.next()){
        devNum = query.value(2).toInt();
        mcuTcpSocketList.insert(query.value(2).toInt(), new QTcpSocket);
        mcuTcpSocketList[devNum]->setObjectName(QString::number(devNum));
        mcuTcpSocketList[devNum]->connectToHost(query.value(0).toString(), query.value(1).toInt());
        connect(mcuTcpSocketList[devNum], SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(networkError(QAbstractSocket::SocketError)));
        connect(mcuTcpSocketList[devNum], SIGNAL(readyRead()), this, SLOT(readDoubleData()));
        connect(mcuTcpSocketList[devNum], SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(connectionStateChanged(QAbstractSocket::SocketState)));
        qDebug()<<"create socket";
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
    //qDebug()<<ip<<port<<dev;
    //qDebug()<<"peer ip"<<transferSocket->peerAddress().toString();
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
#if 0
    for (int i = 0; i < 16; ++i){
        mcuTcpSocketList[i]->disconnectFromHost();
    }

    for (int i = 0; i < 16; ++i){
        mcuTcpSocketList[i]->connectToHost("192.168.0.32", 4000 + i);
    }
#endif
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

void Network::readDoubleData()
{
    struct boardInfo *storage;
    QTcpSocket *tcp=NULL;
    quint8 header;
    QByteArray temp,info;
    quint8 dev;
    tcp = static_cast<QTcpSocket*>(sender());
    QDataStream in(tcp);
    in >> header>>header;
    if (header != 'R'){
        tcp->readAll();
        return;
    }
    while(tcp->bytesAvailable()<66);
    temp = tcp->read(66);
    dev = tcp->objectName().toInt();
    storage = new struct boardInfo;
    qDebug()<<temp;

    /* RsssssssssssssssLLLLLWWWWWeeee
     * R+15*s+5*L+5*W+4*e
     * R数据识别位
     * s条码，
     * L长度
     * W宽度
     * e是extra边缘检测等
     * */
    for (int i = 0; i < 66; ++i) {
        info += temp[(2 * i)];
    }

    info.resize(33);
    qDebug()<<"info"<<info;
    storage->serialNum = info.left(info.indexOf('s')); 
    storage->length = info.mid(info.lastIndexOf('s') + 1, info.indexOf('L') - info.lastIndexOf('s') - 1).toDouble();
    storage->width = info.mid(info.lastIndexOf('L') + 1, info.indexOf('W') - info.lastIndexOf('L') - 1).toDouble();
    storage->boardPerfect = info.at(32) == 'e' ? 'e' : info.at(32) - 0x30;
    storage->devNum = dev;
    mutex.lock();
    QSqlQuery query;
    query.exec(QString("select width, length from detail_information where serial_num ='%1|';").arg(storage->serialNum));
    if (query.next()){
        storage->realWidth = query.value(0).toDouble();
        storage->realLength = query.value(1).toDouble();
    } else {
        storage->realWidth = 0;
        storage->realLength = 0;
    }
    storage->widthMatch = qAbs(storage->width - storage->realWidth) >= 1 ? 0x00 : 0x01;
    storage->lengthMatch = qAbs(storage->length - storage->realLength) >= 1 ? 0x00 : 0x01;

    writeSema.acquire();
    resource.append(storage);
    readSema.release();
    waitCondition.wakeAll();
    mutex.unlock();
    if(socketHashTable[dev] != NULL){
        //socketHashTable[dev]->write(info);
        QDataStream out(socketHashTable[dev]);
        out<<*storage;
        qDebug()<<storage->width<< storage->length<< storage->serialNum<< storage->total<< storage->realWidth<<storage->realLength<<storage->widthMatch<< storage->boardPerfect ;
    }
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
    if (header != 'R'){
        tcp->readAll();
        return;
    }
    while(tcp->bytesAvailable()<29);
    info = tcp->read(29);
    dev = tcp->objectName().toInt();
    storage = new struct boardInfo;
    qDebug()<<info;

    /* RsssssssssssssssLLLLLWWWWWeeee
     * R+15*s+5*L+5*W+4*e
     * R数据识别位
     * s条码，
     * L长度
     * W宽度
     * e是extra边缘检测等
     * */

    storage->serialNum = info.left(info.indexOf('s')); 
    storage->length = info.mid(info.lastIndexOf('s') + 1, info.indexOf('L') - info.lastIndexOf('s') - 1).toInt();
    storage->width = info.mid(info.lastIndexOf('L') + 1, info.indexOf('W') - info.lastIndexOf('L') - 1).toInt();
    storage->boardPerfect = info.at(28) - 0x30;
    storage->devNum = dev;
    QSqlQuery query;
    query.exec(QString("select width, length from detail_information where serial_num ='%1|';").arg(storage->serialNum));
    query.next();
    storage->realWidth = query.value(0).toDouble();
    storage->realLength = query.value(1).toDouble();
    storage->widthMatch = qAbs(storage->width - storage->realWidth) >= 1 ? 0 : 1;
    storage->lengthMatch = qAbs(storage->length - storage->realLength) >= 1 ? 0 : 1;

    writeSema.acquire();
    resource.append(storage);
    readSema.release();
    if(socketHashTable[dev] != NULL){
        //socketHashTable[dev]->write(info);
        QDataStream out(socketHashTable[dev]);
        out<<*storage;
        qDebug()<<storage->width<< storage->length<< storage->serialNum<< storage->total<< storage->realWidth<<storage->realLength<<storage->widthMatch;
    }
}

void Network::networkError(QAbstractSocket::SocketError err)
{
    QTcpSocket *tcp;
    tcp = static_cast<QTcpSocket*>(sender());
    int dev = tcp->objectName().toInt();
    mcuTcpSocketList[dev]->disconnectFromHost();

    qDebug()<<err;
}

void Network::connectionStateChanged(QAbstractSocket::SocketState ss)
{
    quint8 dev;
    dev = static_cast<QTcpSocket*>(sender())->objectName().toInt();

    qDebug()<<dev;
    if (ss == QAbstractSocket::UnconnectedState){
        mcuTcpSocketList[dev]->close();
        
        QMessageBox::warning(0, "连接错误", QString("不能连接到设备%1").arg(dev));
    }
    qDebug()<<mcuTcpSocketList[dev]->localAddress()<<mcuTcpSocketList[dev]->localPort();
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
