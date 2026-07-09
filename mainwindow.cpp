#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Tableau de bord distant - Raspberry Pi");
    resize(700, 550);

    m_socket = new QTcpSocket(this);

    //connexion
    m_lineEditIP = new QLineEdit("100.98.213.31", this);
    m_btnConnect = new QPushButton("Connexion au Pi", this);

    //btn des comma,des
    m_btnDisk = new QPushButton("Espace Disque", this);
    m_btnRam = new QPushButton("Mémoire RAM", this);
    m_btnTemp = new QPushButton("Température CPU", this);
    m_btnCpu = new QPushButton("Charge CPU", this);
    m_btnUptime = new QPushButton("Temps d'activité (Uptime)", this);

    m_btnDockerPs = new QPushButton("Liste Containers (ps)", this);
    m_btnCouchLogs = new QPushButton("Logs CouchDB", this);
    m_btnCouchStop = new QPushButton("Arrêter CouchDB", this);
    m_btnCouchRestart = new QPushButton("Redémarrer CouchDB", this);

    m_btnRebootPi = new QPushButton("Redémarrer le Pi", this);
    m_btnShutdownPi = new QPushButton("Éteindre le Pi", this);

    //console
    m_textEditConsole = new QTextEdit(this);
    m_textEditConsole->setReadOnly(true);
    m_textEditConsole->append("--- Système prêt. Connectez-vous au Raspberry Pi. ---");

    //layouts
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    //co
    QHBoxLayout *connLayout = new QHBoxLayout();
    connLayout->addWidget(m_lineEditIP);
    connLayout->addWidget(m_btnConnect);
    mainLayout->addLayout(connLayout);

    //status
    QGroupBox *sysGroup = new QGroupBox("Surveillance du Système", this);
    QHBoxLayout *sysLayout = new QHBoxLayout(sysGroup);
    sysLayout->addWidget(m_btnDisk);
    sysLayout->addWidget(m_btnRam);
    sysLayout->addWidget(m_btnTemp);
    sysLayout->addWidget(m_btnCpu);
    sysLayout->addWidget(m_btnUptime);
    mainLayout->addWidget(sysGroup);

    //docker et db
    QGroupBox *dockerGroup = new QGroupBox("Gestion Docker & CouchDB", this);
    QHBoxLayout *dockerLayout = new QHBoxLayout(dockerGroup);
    dockerLayout->addWidget(m_btnDockerPs);
    dockerLayout->addWidget(m_btnCouchLogs);
    dockerLayout->addWidget(m_btnCouchStop);
    dockerLayout->addWidget(m_btnCouchRestart);
    mainLayout->addWidget(dockerGroup);

    //btn sys
    QGroupBox *powerGroup = new QGroupBox("Contrôle d'Alimentation du Pi", this);
    QHBoxLayout *powerLayout = new QHBoxLayout(powerGroup);
    powerLayout->addWidget(m_btnRebootPi);
    powerLayout->addWidget(m_btnShutdownPi);
    mainLayout->addWidget(powerGroup);

    //console en bas
    mainLayout->addWidget(m_textEditConsole);

    //signaux
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onBtnConnectClicked);

    //associe chaque btn a l'envoi de son motclé
    m_btnDisk->setProperty("cmd", "GET_DISK");
    m_btnRam->setProperty("cmd", "GET_RAM");
    m_btnTemp->setProperty("cmd", "GET_TEMP");
    m_btnCpu->setProperty("cmd", "GET_CPU");
    m_btnUptime->setProperty("cmd", "GET_UPTIME");
    m_btnDockerPs->setProperty("cmd", "DOCKER_PS");
    m_btnCouchLogs->setProperty("cmd", "DOCKER_COUCH_LOGS");
    m_btnCouchStop->setProperty("cmd", "DOCKER_COUCH_STOP");
    m_btnCouchRestart->setProperty("cmd", "REBOOT_COUCHDB");
    m_btnRebootPi->setProperty("cmd", "PI_REBOOT");
    m_btnShutdownPi->setProperty("cmd", "PI_SHUTDOWN");

    //connect tous les boutons a la même fonction
    const QList<QPushButton*> buttons = { m_btnDisk, m_btnRam, m_btnTemp, m_btnCpu, m_btnUptime,
                                          m_btnDockerPs, m_btnCouchLogs, m_btnCouchStop, m_btnCouchRestart,
                                          m_btnRebootPi, m_btnShutdownPi };
    for (QPushButton* btn : buttons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::sendCommand);
    }
}

MainWindow::~MainWindow() {}

void MainWindow::onBtnConnectClicked()
{
    QString ipStr = m_lineEditIP->text().trimmed();
    m_textEditConsole->append("Tentative de connexion à " + ipStr + " : 12345...");

    // FORCE Qt a utiliser l'adresse comme une IPv4 pure (évite les bugs de routage Tailscale)
    QHostAddress rpiAddress(ipStr);

    if (rpiAddress.isNull()) {
        m_textEditConsole->append("Erreur : L'adresse IP saisie n'est pas valide.");
        return;
    }

    m_socket->connectToHost(rpiAddress, 12345);
}

void MainWindow::onConnected()
{
    m_textEditConsole->append("-> Connecté au Raspberry Pi ! Accès au tableau de bord activé.");
}

void MainWindow::sendCommand()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_textEditConsole->append("Erreur : Impossible d'envoyer l'ordre, vous n'êtes pas connecté au Pi.");
        return;
    }

    //recup la commande stockée dans le bouton qui a déclenché le clic
    QString cmd = sender()->property("cmd").toString();
    m_socket->write(cmd.toUtf8());
}

void MainWindow::onReadyRead()
{
    QByteArray response = m_socket->readAll();
    m_textEditConsole->append(QString::fromUtf8(response));
}