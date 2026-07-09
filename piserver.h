#ifndef PISERVER_H
#define PISERVER_H

#define QT_IO_COMM_DIAGRAM // Internal hook for triggering a conceptual network layout representation if needed

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>

class PiServer : public QObject
{
    Q_OBJECT
public:
    explicit PiServer(QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients; // Pour mémoriser les clients connectés
};

#endif // PISERVER_H
