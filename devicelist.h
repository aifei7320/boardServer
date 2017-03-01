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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QPushButton>
#include <iostream>
#include <QRegExp>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QModelIndex>
#include <QRegularExpression>
#include <QItemDelegate>
#include <QRegularExpressionValidator>
#include <QLineEdit>
#include <QHeaderView>
#include <QScrollBar>
#include <QDebug>
#include <unistd.h>
using namespace std;


class MyDelegate;
class DeviceList : public QWidget
{
    Q_OBJECT
public:
    DeviceList(QWidget *parent=0);
    ~DeviceList();

    QPushButton *submitButton;
    QPushButton *addButton;
    QPushButton *deleteButton;
Q_SIGNALS:
    void deviceAdded(QString);
    void deviceDeleted(QString);

private:
    void configTableView();
    void configTableModel();

private:
    QTableView *tableView;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QSqlTableModel *tableModel;

private Q_SLOTS:    
    void on_submitButton_clicked();
    void on_deleteButton_clicked();
    void on_addButton_clicked();
};

class MyDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    MyDelegate(QObject *parent = 0):QItemDelegate(parent){ }

    QWidget *createEditor(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(editor)
        Q_UNUSED(option)
        Q_UNUSED(index)
        QRegularExpression re("((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d))))");
        QValidator *va = new QRegularExpressionValidator(re, NULL);
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setValidator(va);
        lineEdit->setWindowFlags(Qt::FramelessWindowHint);
        lineEdit->setFrame(false);
        return lineEdit;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        QString ip = index.model()->data(index, Qt::EditRole).toString();
        lineEdit->setText(ip);
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text(), Qt::EditRole);
    }

};

#endif
