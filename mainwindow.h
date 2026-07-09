#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBtnConnectClicked();
    void onReadyRead();
    void onConnected();

    //slots pour envoyer chaque commande au Pi
    void sendCommand();

private:
    QTcpSocket  *m_socket;
    QLineEdit   *m_lineEditIP;
    QPushButton *m_btnConnect;
    QTextEdit   *m_textEditConsole;

    //btns rangés par catégories
    QPushButton *m_btnDisk;
    QPushButton *m_btnRam;
    QPushButton *m_btnTemp;
    QPushButton *m_btnCpu;
    QPushButton *m_btnUptime;

    QPushButton *m_btnDockerPs;
    QPushButton *m_btnCouchLogs;
    QPushButton *m_btnCouchStop;
    QPushButton *m_btnCouchRestart;

    QPushButton *m_btnRebootPi;
    QPushButton *m_btnShutdownPi;
};
#endif // MAINWINDOW_H