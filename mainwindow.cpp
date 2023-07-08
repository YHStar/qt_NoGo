#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSoundEffect>
QSoundEffect *music = new QSoundEffect();
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 560);

    resize(800, 560);
    setWindowTitle("NoGo");

    QPalette pa(this->palette());
    QImage img=QImage(":/w");
    img=img.scaled(this->size());
    QBrush *pic=new QBrush(img);
    pa.setBrush(QPalette::Window,*pic);
    this->setPalette(pa);

    //播放音频

    music->setSource(QUrl::fromLocalFile(":/music.wav"));
    music->setLoopCount(QSoundEffect::Infinite);  //设置无限循环
    music->setVolume(0.5f);  //设置音量，在0到1之间
    music->play();


    offButton = new QPushButton("离线版", this);
    onButton = new QPushButton("在线版", this);
    exitButton = new QPushButton("退出", this);
    Cp=new QPushButton("暂停",this);
    offButton->setGeometry(300,300,200,60);
    onButton->setGeometry(300,375,200,60);
    exitButton->setGeometry(300,450,200,60);
    Cp->setGeometry(40,40,40,20);
    offButton->setFont(QFont("黑体",16));
    onButton->setFont(QFont("黑体",16));
    exitButton->setFont(QFont("黑体",30));
    connect(this->offButton, SIGNAL(clicked()), this, SLOT(off()));
    connect(this->onButton, SIGNAL(clicked()), this,SLOT(on()));
    connect(this->exitButton, SIGNAL(clicked()), this, SLOT(exitb()));
    connect(this->Cp, SIGNAL(clicked()), this, SLOT(change()));
}
void MainWindow::change()
{
    bool tmp=music->isPlaying();
    if(!tmp)
    {
        music->play();
        Cp->setText("暂停");
    }
    if(tmp)
    {
        music->stop();
        Cp->setText("播放");
    }
}
void MainWindow::exitb()
{
    exit(0);
}
void MainWindow::off()
{
    QProcess process(this);
    process.startDetached("./NoGo.exe");
}
void MainWindow::on()
{
    QProcess process(this);
    process.startDetached("./NoGo_online.exe");
}
MainWindow::~MainWindow()
{
    delete ui;
}

