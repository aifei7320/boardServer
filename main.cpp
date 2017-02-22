/*************************************************************************
    > File Name: main.cpp
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年02月21日 星期二 13时30分44秒
 ************************************************************************/

#include <iostream>
#include <unistd.h>
using namespace std;

#include <QApplication>
#include "network.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    Network network;

    return app.exec();
}
