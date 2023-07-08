#include "socketthread.h"

ServerSocketThread::ServerSocketThread(SOCKET mListen,QWidget *parent) : QThread(parent)
{
    this->mListen = mListen;
    this->parent = parent;
}
ServerSocketThread::~ServerSocketThread()
{
    qDebug() << "SocketThread 析构函数";
    closesocket(mListen);
    emit isClose();
}
void ServerSocketThread::run()
{
    sockaddr_in clientAddr;
    int size = sizeof(clientAddr);
    while(!isInterruptionRequested()){
        //每次接受新客户端将之前的地址信息清0
        memset(&clientAddr,0,sizeof(clientAddr));
        SOCKET client = ::accept(mListen,(sockaddr*)&clientAddr,&size);
        QString msg = QString("b");
        //新客户端连接，通知 UI 更新界面
        emit isMsg(msg);
        //开启新线程和客户端进行通信
        ServerMsgThread* msgThread = new ServerMsgThread(client,clientAddr,parent);
        msgThread->start();

        connect(msgThread,&ServerMsgThread::isMsg,this,[=](QString msg){
            //转发消息给 UI进程，UI进行界面更新
            emit isMsg(msg);
        });
        connect(this,&ServerSocketThread::isClose,this,[=](){
            msgThread->terminate();
            msgThread->quit();
            delete msgThread;
        });
        connect(this,&ServerSocketThread::misSend,msgThread,&ServerMsgThread::dealSend,Qt::DirectConnection);
    }
}
void ServerSocketThread::dealSend(QString msg)
{
    mMsgi=msg;
    emit misSend(mMsgi);
}
