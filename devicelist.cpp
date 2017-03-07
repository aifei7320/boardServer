/*************************************************************************
    > File Name: devicelist.cpp
    > Author: zxf
    > Mail: zhengxiaofeng333@163.com 
    > Created Time: 2017年03月01日 星期三 10时01分57秒
 ************************************************************************/

#include <iostream>
#include <unistd.h>
#include "devicelist.h"
using namespace std;

//extern QSqlDatabase mySqlDb;

DeviceList::DeviceList(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout;
    buttonLayout = new QHBoxLayout;

    tableModel = new QSqlTableModel;
    configTableModel();

    tableView = new QTableView;
    configTableView();

    addButton = new QPushButton;
    addButton->setText("添加");

    deleteButton = new QPushButton;
    deleteButton->setText("删除");

    submitButton = new QPushButton;
    submitButton->setText("提交");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(submitButton);

    connect(addButton, SIGNAL(clicked()), this, SLOT(on_addButton_clicked()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(on_deleteButton_clicked()));
    connect(submitButton, SIGNAL(clicked()), this, SLOT(on_submitButton_clicked()));

    mainLayout->addWidget(tableView);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

DeviceList::~DeviceList()
{
    delete submitButton;
    delete deleteButton;
    delete addButton;
    delete buttonLayout;
    delete mainLayout;
}

void DeviceList::configTableView()
{
    MyDelegate *my = new MyDelegate(Q_NULLPTR);
    tableView->setModel(tableModel);
    tableView->setShowGrid(true);
    tableView->setGridStyle(Qt::SolidLine);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setItemDelegateForColumn(2, my);
    tableView->hideColumn(0);
}

void DeviceList::configTableModel()
{
    tableModel->setTable("deviceList");

    tableModel->setHeaderData(0, Qt::Horizontal, "编号");
    tableModel->setHeaderData(1, Qt::Horizontal, "名称");
    tableModel->setHeaderData(2, Qt::Horizontal, "ip地址");
    tableModel->setHeaderData(3, Qt::Horizontal, "端口号");
    tableModel->setHeaderData(4, Qt::Horizontal, "设备编号");

    tableModel->select();
}

void DeviceList::on_addButton_clicked()
{
    QSqlQuery query;
    query.exec("insert into deviceList (name) values('')");
    tableModel->select();
    QScrollBar *bar = tableView->verticalScrollBar();
    bar->setValue(bar->maximum());
    tableView->selectRow(tableModel->rowCount() - 1);
}

void DeviceList::on_deleteButton_clicked()
{
    QSqlQuery query;
    QModelIndex current = tableView->currentIndex();
    query.exec(QString("delete from deviceList where id = %1").arg(current.sibling(current.row(), 0).data().toInt()));
    tableModel->select();
}
void DeviceList::on_submitButton_clicked()
{
    tableModel->select();
    qDebug()<<tableModel->lastError();
}
