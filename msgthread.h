#ifndef MSGTHREAD_H
#define MSGTHREAD_H

#pragma once

#include <QWidget>
#include <QThread>
#include <winsock2.h>
#include <QTime>
#include <QDebug>
#pragma comment(lib,"ws2_32.lib")

class ClientMsgThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientMsgThread(SOCKET mSocket,QWidget *parent = nullptr);
    void run();

signals:
    void isMsg(QString msg);
    void isClose();

public slots:
    void dealSend(QString msg);
private:
    QString mMsg;
    SOCKET mSocket;
    bool mIsSend =false;
};

class ServerMsgThread : public QThread
{
    Q_OBJECT
public:
    explicit ServerMsgThread(SOCKET mClient,sockaddr_in mAddr,QWidget *parent = nullptr);
    ~ServerMsgThread();
    void run();
signals:
    void isMsg(QString msg);//客户端socket关闭了 客户端发消息来了

public slots:
    void dealSend(QString msg);
private:
    //和客户端通信的 socket
    SOCKET mClient;
    //客户端地址
    sockaddr_in mAddr;
    QString resp;
    bool cmIsSend=false;
};



#endif // MSGTHREAD_H
