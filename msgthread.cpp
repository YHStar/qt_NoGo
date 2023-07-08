#include "msgthread.h"

ClientMsgThread::ClientMsgThread(SOCKET mSocket,QWidget *parent) : QThread(parent)
{
    this->mSocket = mSocket;
}

void ClientMsgThread::run(){

    char buf[10] = {0};
    while(true){
        if(mIsSend){
            qDebug() << "msg:"<< mMsg;
            //发送
            ::send(mSocket,mMsg.toUtf8().data(),mMsg.length()+1,0);
            //等待服务端回应
            int ret = ::recv(mSocket,buf,10,0);
            //服务端关闭
            if( ret == 0){
                emit isClose();
                return;
            }
            qDebug() << "buf:" << buf ;
            QString msg = QString("%1").arg(buf);
            emit isMsg(msg);
            mIsSend = false;
        }
    }

}
void ClientMsgThread::dealSend(QString msg){
    mIsSend = true;
    mMsg = msg;
}
ServerMsgThread::ServerMsgThread(SOCKET mClient,sockaddr_in mAddr,QWidget *parent) : QThread(parent)
{
    this->mClient = mClient;
    this->mAddr = mAddr;
}
ServerMsgThread::~ServerMsgThread(){
    qDebug() << "msgThread 析构函数";
    int ret = closesocket(mClient);
    if( ret == 0 ){
        qDebug()<< "关闭成功";
    }else{
        qDebug()<< "关闭失败";
    }
}
void ServerMsgThread::run(){
    //inet_addr点分十进制转网络ip地址 ,inet_ntoa网络转点分十进制
    while(true){
        char buf[10] = {0};
        //阻塞等待 接受信息
        int ret = recv(mClient,buf,10,0);
        if(ret == 0){
            emit isMsg(QString("c"));
            break;
        }
        //接受到消息，通知UI 界面更新
        QString msg = QString("%1").arg(buf);
        emit isMsg(msg);
        while(1)
        {
            if(cmIsSend)
            {
                qDebug() << "给客户端发送消息：" << resp;
                send(mClient,resp.toUtf8().data(),resp.length()+1,0);
                cmIsSend=false;
                break;
            }
        }
    }
}
void ServerMsgThread::dealSend(QString msg){
    cmIsSend = true;
    resp = msg;
}
