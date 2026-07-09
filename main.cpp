#include <QCoreApplication>
#include "piserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // On instancie notre serveur
    PiServer server;

    return a.exec(); // Lance la boucle infinie pour que le réseau reste actif
}
