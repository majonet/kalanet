#include <QCoreApplication>
#include "Server.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    Server server;
    if (!server.start(12345)) {
        qDebug() << "Failed to start server on port 12345";
        return 1;
    }
    qDebug() << "KalaNet Server started on port 12345";
    return app.exec();
}