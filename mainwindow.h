#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void change();
    void exitb();
    void off();
    void on();

private:
    Ui::MainWindow *ui;
    QPushButton *offButton,*onButton,*exitButton,*Cp;
};
#endif // MAINWINDOW_H
