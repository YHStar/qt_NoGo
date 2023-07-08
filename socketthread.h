#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#pragma once
#include <QWidget>
#include <QThread>
#include <winsock2.h>
#include"msgthread.h"
#include <stdlib.h>

class ServerSocketThread : public QThread
{
    Q_OBJECT
public:
    explicit ServerSocketThread(SOCKET mListen,QWidget *parent = nullptr);
    void run();
    ~ServerSocketThread();
signals:
    void isMsg(QString msg);//新客户端连接了 客户端socket关闭了 客户端发消息来了 都使用此信号
    void isClose();//服务端 关闭信号
    void misSend(QString msg);
public slots:
    void dealSend(QString msg);
private:
    SOCKET mListen;
    QWidget *parent;
    QString mMsgi;
};



#endif // SOCKETTHREAD_H
