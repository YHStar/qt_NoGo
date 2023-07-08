#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QWidget>
#include<QPushButton>
#include<QVector>
#include"Item.h"
#include<QApplication>
#include<QStyle>
#include<QPalette>
#include<QPainter>
#include<QPen>
#include<QBrush>
#include<QMouseEvent>
#include<QPainterPath>
#include<QThread>
#include<QLabel>
#include<QFile>
#include<QDataStream>
#include<QXmlStreamWriter>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
   MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int firsthand;
    const char over[3][4] = { "AI", "", "你" };
    int tempstart = 0;
    int overi = 0;
    int chessboard[9][9]={{0}};
    int tempbackXUser, tempbackYUser, tempbackXAI, tempbackYAI;
    class treeNode;
    void mcts();
    void DrawItem();
    QVector<Item>m_items;
    QLabel*label1;
    int m_bIsBlackTun;

    void Failedback();
    void save();
    void load();
    void mousePressEvent(QMouseEvent*event);
    void paintEvent(QPaintEvent*event);
    void on_pushButton_clicked();
private:
    Ui::MainWindow *ui;

    QPushButton*gameButton;
    QPushButton*exitButton;
    QPushButton* pushButton1;
    QPushButton* pushButton2;
    QPushButton* pushButton3;

public slots:
    void gameButton_Clicked();
    void exitButton_Clicked();
    void pushButton1_Clicked();
    void pushButton2_Clicked();
    void pushButton3_Clicked();
};
#endif // MAINWINDOW_H
