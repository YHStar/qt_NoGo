#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include<QWidget>
#include<QPushButton>
#include<QVector>
#include"Item.h"
#include<QPalette>
#include<QMouseEvent>
#include<QPainterPath>
#include<QPainter>
#include<QThread>
#include "msgthread.h"
#include "socketthread.h"
#include <QDebug>
#include<QLineEdit>
#include<QListWidget>
#include<QLabel>
#include<QString>
#include<ws2tcpip.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //server
    void closeEvent(QCloseEvent *event);

    int firsthand;
    int chessboard[9][9]={{0}};
    void DrawItem();
    QVector<Item>m_items;
    QLabel *label1;
    int mycolor,yourcolor;

    void Failedback();
    void mousePressEvent(QMouseEvent*event);
    void paintEvent(QPaintEvent*event);
    void on_pushButton_clicked();
    void show_nogo();
    void rec_msg(QString msg);
    void Clean();

private:
    Ui::MainWindow *ui;
    WSADATA mWasData;
    QPushButton* serverButton;
    QPushButton* clientButton;
    QPushButton* exitButton;
    QPushButton* clientConnect;
    QLineEdit *line;
    QLabel *label;
    QListWidget *listw;
    //client
    SOCKET mSocket;
    ClientMsgThread* mMsgThread;
    QString mAddrStr;
    //server
    SOCKET mListen = INVALID_SOCKET;
    ServerSocketThread *mSocketThread;

public slots:
    void serverButton_Clicked();
    void clientButton_Clicked();
    void exitButton_Clicked();
    void clientConnect_Clicked();
signals:
    //client
    void isSend(QString msg);
};
#endif // MAINWINDOW_H
