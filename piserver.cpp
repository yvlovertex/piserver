#include "piserver.h"
#include <QDebug>

PiServer::PiServer(QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer(this);

    // On connecte le signal de nouvelle connexion à notre méthode
    connect(m_server, &QTcpServer::newConnection, this, &PiServer::onNewConnection);

    // Le serveur écoute sur le port 12345 et accepte toutes les IP (Any)
    if (m_server->listen(QHostAddress::Any, 12345)) {
        qDebug() << "=== Serveur en ligne ===";
        qDebug() << "En attente de connexions sur le port 12345...";
    } else {
        qDebug() << "Erreur de démarrage du serveur :" << m_server->errorString();
    }
}

void PiServer::onNewConnection()
{
    // On récupère la socket du client qui vient de se connecter
    QTcpSocket *clientSocket = m_server->nextPendingConnection();
    m_clients.append(clientSocket);

    qDebug() << "Un client s'est connecté depuis l'adresse :" << clientSocket->peerAddress().toString();

    // On écoute les messages de CE client spécifique
    connect(clientSocket, &QTcpSocket::readyRead, this, &PiServer::onReadyRead);
    // On écoute aussi s'il se déconnecte pour faire le ménage
    connect(clientSocket, &QTcpSocket::disconnected, this, &PiServer::onClientDisconnected);

    clientSocket->write("Bienvenue sur le serveur de controle du Pi !\n");
}

void PiServer::onReadyRead()
{
    // On récupère le client qui a envoyé le message
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    // On lit tout le message reçu
    QByteArray data = socket->readAll();
    QString commande = QString::fromUtf8(data).trimmed(); // .trimmed() enlève les espaces et \n

    qDebug() << "Commande recue du client :" << commande;

    // --- ICI ON AJOUTE LES ORDRES ---
    if (commande == "GET_DISK") {
#ifdef Q_OS_WIN
        // Commande pour tester si tu es sous Windows actuellement
        QProcess proc;
        proc.start("wmic", QStringList() << "logicaldisk" << "get" << "size,freespace,caption");
#else \
    // Commande classique Linux/Mac
        QProcess proc;
        proc.start("df", QStringList() << "-h" << "/");
#endif
        proc.waitForFinished();
        socket->write(proc.readAllStandardOutput());
    }
    else if (commande == "REBOOT_COUCHDB") {
        // Commande simulée ou réelle pour ton docker
        // QProcess::startDetached("docker restart couchdb");
        socket->write("Ordre recu : Tentative de redemarrage de CouchDB...\n");
    }
    else {
        socket->write("Commande inconnue. Essayez 'GET_DISK' ou 'REBOOT_COUCHDB'.\n");
    }
}

void PiServer::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        qDebug() << "Client deconnecté.";
        m_clients.removeOne(socket);
        socket->deleteLater();
    }
}
