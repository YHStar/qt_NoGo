#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Item.h"
#include "socketthread.h"
#include "msgthread.h"

#include <QSoundEffect>
int cnnt=1;

int lDist = 60;
bool visited_by_air_judge[9][9] = { {false} };
int dx[4] = { -1, 0, 1, 0 };
int dy[4] = { 0, -1, 0, 1 };
bool buttonFlage=1;
bool tempstart = 0;
QLabel* loss_label1;
QLabel* loss_label2;
bool c=0,s=0;

bool inBoard_judge(int x, int y)
{
    return 0 <= x && x < 9 && 0 <= y && y < 9;
}
bool air_judge(int board[9][9], int x, int y)
{
    visited_by_air_judge[x][y] = true; // 标记，表示这个位置已经搜过有无气了
    bool flag = false;
    for (int dir = 0; dir < 4; ++dir)
    {
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)) // 界内
        {
            if (board[x_dx][y_dy] == 0) // 旁边这个位置没有棋子
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) // 旁边这个位置是没被搜索过的同色棋
                if (air_judge(board, x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}
bool put_available(int board[9][9], int x, int y, int color)
{
    if (!inBoard_judge(x, y))
        return false;
    if (board[x][y]) // 如果这个点本来就有棋子
        return false;

    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); // 重置

    if (!air_judge(board, x, y)) // 如果下完这步这个点没气了,说明是自杀步，不能下
    {
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; ++i) // 判断下完这步周围位置的棋子是否有气
    {
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)) // 在棋盘内
        {
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) // 对于有棋子的位置（标记访问过避免死循环）
                if (!air_judge(board, x_dx, y_dy))                      // 如果导致(x_dx,y_dy)没气了，则不能下
                {
                    board[x][y] = 0; // 回溯
                    return false;
                }
        }
    }
    board[x][y] = 0; // 回溯
    return true;
}
void MainWindow::show_nogo()
{
    listw->show();
    delete label;
    QFont f1;
    f1.setFamily("Cascadia Code");
    f1.setPixelSize(115);
    label1=new QLabel(this);
    label1->setText("NoGo");
    label1->move(0,40);
    label1->resize(300,150);
    label1->setFont(f1);
    label1->setScaledContents(true);
    label1->show();
    QPalette pa(this->palette());
    QImage img=QImage(":/bg_c");
    img=img.scaled(this->size());
    QBrush *pic=new QBrush(img);
    pa.setBrush(QPalette::Window,*pic);
    this->setPalette(pa);
    return;
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 560);

    resize(800, 560);
    setWindowTitle("NoGo");
    WSAStartup(MAKEWORD(2,2),&mWasData);

    QPalette pa(this->palette());
    QImage img=QImage(":/w");
    img=img.scaled(this->size());
    QBrush *pic=new QBrush(img);
    pa.setBrush(QPalette::Window,*pic);
    this->setPalette(pa);

    //播放音频
    QSoundEffect *music = new QSoundEffect();
    music->setSource(QUrl::fromLocalFile(":/music.wav"));
    music->setLoopCount(QSoundEffect::Infinite);  //设置无限循环
    music->setVolume(0.5f);  //设置音量，在0到1之间
    music->play();

    serverButton = new QPushButton("创建棋局（后手）", this);
    clientButton = new QPushButton("加入棋桌（先手）", this);
    exitButton = new QPushButton("退出", this);
    serverButton->setGeometry(300,300,200,60);
    clientButton->setGeometry(300,375,200,60);
    exitButton->setGeometry(300,450,200,60);
    serverButton->setFont(QFont("黑体",16));
    clientButton->setFont(QFont("黑体",16));
    exitButton->setFont(QFont("黑体",30));
    connect(this->serverButton, SIGNAL(clicked()), this, SLOT(serverButton_Clicked()));
    connect(this->clientButton, SIGNAL(clicked()), this,SLOT(clientButton_Clicked()));
    connect(this->exitButton, SIGNAL(clicked()), this, SLOT(exitButton_Clicked()));
    listw=new QListWidget(this);
    listw->setGeometry(QRect(40,180,190,200));
    listw->hide();
    label=new QLabel(this);
    label->setFont(QFont("黑体",16));
    label->hide();

    m_items.clear();
    memset(chessboard,0,sizeof(chessboard));
    return ;
}
void MainWindow::closeEvent(QCloseEvent *event){
    if(s)
        if(mSocketThread != nullptr){
            mSocketThread->requestInterruption();
            mSocketThread->terminate();
            qDebug() << "释放 mSocketThread";
            mSocketThread->quit();
            mSocketThread->wait();
            delete mSocketThread;
        }
}
void MainWindow::serverButton_Clicked()
{
    delete serverButton;
    delete clientButton;
    delete exitButton;
    label->setGeometry(300,400,200,60);
    label->show();
    s=1;
    if(INVALID_SOCKET != mListen)
        return;

    //端口校验
    int port = 9898;

    //创建套接字，正确返回有效的套接字
    mListen = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    int error;
    if(mListen == INVALID_SOCKET){
        error = WSAGetLastError();
        qDebug() << "socket error:" << error;
        return;
    }

    sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons((u_short)port);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;//服务端可以这样写，主机的任意合适的地址
    //绑定套接字和地址信息
    int ret = bind(mListen,(sockaddr*)&addr,sizeof(addr));
    if( ret == SOCKET_ERROR){
        error = WSAGetLastError();
        qDebug() << "bind error:" << error;
        return;
    }

    //端口监听
    ret = ::listen(mListen,SOMAXCONN);
    if( ret == SOCKET_ERROR){
        error = WSAGetLastError();
        qDebug() << "listen error:" << error;
        return;
    }

    label->setText("启动成功，等待连接");
    label->setStyleSheet("color:orange;");

    //启动后台进程和客户端进行连接，必须这样，因为ui线程不能被阻塞！
    mSocketThread = new ServerSocketThread(mListen);
    mSocketThread->start();

    //信号槽连接
    connect(mSocketThread,&ServerSocketThread::isMsg,this,&MainWindow::rec_msg);
    connect(this,&MainWindow::isSend,mSocketThread,&ServerSocketThread::dealSend);
}
void MainWindow::rec_msg(QString msg)
{
    buttonFlage=0;
    if(msg[0]=='b')
    {
        buttonFlage=1;
        show_nogo();
        listw->addItem("比赛开始！您为后手（白子）");
        mycolor=1;
        yourcolor=0;
        tempstart=1;
    }
    else if(msg[0]=='c')
    {
        listw->addItem("对方退出了游戏");
    }
    else if(msg[0]=='s')
    {
        listw->addItem("对方首着下在了中间，你赢了");
    }
    else if(msg[0]=='w')
    {
        msg[0]=msg[1];
        msg[1]=msg[2];
        msg[2]='\0';
        int tmp_msg=msg.toInt();
        qDebug()<<tmp_msg<<" "<<msg<<'\n';
        QPoint point;
        point.setX(tmp_msg/10);
        point.setY(tmp_msg%10);
        Item item1(point,yourcolor);
        m_items.append(item1);
        chessboard[point.x()][point.y()] = -1;
        QString add_msg = QString("[对手]第%1着 下在第%2列 第%3行").arg(cnnt++).arg(tmp_msg/10).arg(tmp_msg%10);
        listw->addItem(add_msg);
        Failedback();
        loss_label1->setText("你赢了!");
        loss_label1->setStyleSheet("color:blue;");
        loss_label1->show();
        Clean();
    }
    else
    {
        int tmp_msg=msg.toInt();
        qDebug()<<tmp_msg<<" "<<msg<<'\n';
        QPoint point;
        point.setX(tmp_msg/10);
        point.setY(tmp_msg%10);
        Item item1(point,yourcolor);
        m_items.append(item1);
        chessboard[point.x()][point.y()] = -1;
        QString add_msg = QString("[对手]第%1着 下在第%2列 第%3行").arg(cnnt++).arg(tmp_msg/10).arg(tmp_msg%10);
        listw->addItem(add_msg);
    }
}
void MainWindow::clientButton_Clicked()
{
    delete serverButton;
    delete clientButton;
    delete exitButton;
    label->setGeometry(300,300,200,60);
    label->setText("请输入IP地址");
    label->show();
    clientConnect=new QPushButton("连接",this);
    clientConnect->setGeometry(325,450,150,60);
    clientConnect->show();
    connect(clientConnect,SIGNAL(clicked()),this,SLOT(clientConnect_Clicked()));
    line=new QLineEdit(this);
    line->setGeometry(300,370,200,60);
    line->show();
}
void MainWindow::clientConnect_Clicked()
{
    c=1;
    mSocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(mSocket == INVALID_SOCKET){
        label->setText(QString("连接失败！！！"));
        label->setStyleSheet("color:red;");
        c=0;
        return;
    }
    QString QStmp;
    QStmp=line->text();
    qDebug()<<"Done";
    char* ip;
    int port=9898;
    QByteArray ba = QStmp.toLatin1();
    ip=ba.data();
    SOCKADDR_IN addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons((u_short)port);
    unsigned long tmp_addr;
    inet_pton(PF_INET,ip,&tmp_addr);
    addr.sin_addr.S_un.S_addr = tmp_addr;//点分十进制转ip地址
    int val = ::connect(mSocket,(SOCKADDR*)&addr,sizeof(addr));
    int error;
    if( val == SOCKET_ERROR){
        label->setText(QString("连接失败！！！"));
        label->setStyleSheet("color:red;");
        error = WSAGetLastError();
        qDebug() << "connect error:" << error;
        c=0;
        return;

    }
    label->setText(QString("连接成功"));
    label->setStyleSheet("color:orange;");
    if(c)
    {
        delete line;
        delete clientConnect;
        show_nogo();
        listw->addItem("比赛开始！您为先手（黑子）");
        mycolor=0;
        yourcolor=1;
        buttonFlage=0;
    }

    //创建处理和服务端通信的线程
    mMsgThread = new ClientMsgThread(mSocket);
    mMsgThread->start();
    //信号槽连接
    connect(this,&MainWindow::isSend,mMsgThread,&ClientMsgThread::dealSend);
    connect(mMsgThread,&ClientMsgThread::isMsg,this,&MainWindow::rec_msg);
}
void MainWindow::exitButton_Clicked()
{
    exit(0);
}
void MainWindow::DrawItem()
{

    QPainter painter(this);
    QPen pen;
    pen.setColor(Qt::transparent);
    painter.setPen(pen);
    for(int i=0;i<m_items.size();i++)
    {
        Item item=m_items[i];

        if(!item.m_bBlack)
            painter.setBrush(Qt::black);
        else
            painter.setBrush(Qt::white);

        QPoint ptCenter((item.m_pt.x()*lDist)+280,(item.m_pt.y()*lDist)+40);
        painter.drawEllipse(ptCenter,lDist/2,lDist/2);
    }
}
void MainWindow::Failedback()
{
    //label1->hide();

    loss_label1=new QLabel(this);
    loss_label2=new QLabel(this);
    loss_label1->move(280,200);
    loss_label2->move(280,260);
    loss_label1->resize(200,200);
    loss_label2->resize(400,200);
    QFont f;
    f.setPixelSize(50);
    f.setFamily("黑体");
    loss_label1->setFont(f);
    loss_label2->setFont(f);
    loss_label1->hide();
    loss_label2->hide();
}
void MainWindow::Clean()
{
    if(c)
    {
        ::closesocket(mSocket);
        //winsock 释放
        WSACleanup();
        if(mMsgThread!=nullptr)
            mMsgThread->quit();
        c=0;
    }
    if(s)
    {
        if(mSocketThread != nullptr){
            mSocketThread->requestInterruption();
            mSocketThread->terminate();
            qDebug() << "释放 mSocketThread";
            mSocketThread->quit();
            mSocketThread->wait();
            delete mSocketThread;
        }
        ::closesocket(mListen);
        WSACleanup();
        s=0;
    }
}
void MainWindow::mousePressEvent(QMouseEvent*event)
{
    if(buttonFlage)
        return;
    buttonFlage=1;

    QPoint pt1;
    int chess_x=event->pos().x();
    int chess_y=event->pos().y();
    if(chess_x<=250)
        return;

    int tmpx=chess_x-280;
    int tmpy=chess_y-40;
    for(int i=0;i<=8;i++)
    {
        if(tmpx<=30*(2*i+1))
        {
            tmpx=60*i;
            break;
        }
    }
    for(int j=0;j<=8;j++)
    {
        if(tmpy<=30*(2*j+1))
        {
            tmpy=60*j;
            break;
        }
    }
    pt1.setX(tmpx/lDist);
    pt1.setY(tmpy/lDist);

    if (tempstart == 0 && pt1.x() == 4 && pt1.y() == 4)
    {
        Failedback();
        emit isSend("s");
        loss_label1->setText("禁止首着");
        loss_label2->setText("落于正中心");
        loss_label1->setStyleSheet("color:red;");
        loss_label2->setStyleSheet("color:red;");
        loss_label1->show();
        loss_label2->show();
        return;
    }
    tempstart = 1;
    if (!chessboard[pt1.x()][pt1.y()])
    {
        Item item1(pt1,mycolor);
        m_items.append(item1);
        int int_msg=tmpx/lDist*10+tmpy/lDist;
        QString qstr_msg=QString::number(int_msg);
        QString add_msg = QString("[自己]第%1着 下在第%2列 第%3行").arg(cnnt++).arg(tmpx/lDist).arg(tmpy/lDist);
        listw->addItem(add_msg);
        if (!put_available(chessboard, pt1.x(), pt1.y(), 1))
        {
            Failedback();
            loss_label1->setText("你输了!");
            loss_label1->setStyleSheet("color:red;");
            loss_label1->show();
            qstr_msg='w'+qstr_msg;
            emit isSend(qstr_msg);
            repaint();
            QThread::msleep(2000);
            Clean();
        }
        else
        emit isSend(qstr_msg);
        chessboard[pt1.x()][pt1.y()] = 1;
        return;
    }


}
void MainWindow::paintEvent(QPaintEvent*event)
{
    DrawItem();
    update();
}
MainWindow::~MainWindow()
{
    if(c)
    {
        ::closesocket(mSocket);
        //winsock 释放
        WSACleanup();
        if(mMsgThread!=nullptr)
            mMsgThread->quit();
    }
    if(s)
    {
        ::closesocket(mListen);
        WSACleanup();
    }
    delete ui;
}

