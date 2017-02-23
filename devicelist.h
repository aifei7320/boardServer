/*************************************************************************
    > File Name: devicelist.h
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年02月22日 星期三 20时59分14秒
 ************************************************************************/

#ifndef _DEVICELIST_H_
#define _DEVICELIST_H_

#include <QWidget>
#include <QTableView>
#include <QItemDelegate>
#include <QString>
#include <QPushButton>
#include <iostream>
#include <unistd.h>
using namespace std;


class DeviceList : public QWidget
{
    Q_OBJECT
public:
    DeviceList(QWidget *parent=0);
}

#endif
