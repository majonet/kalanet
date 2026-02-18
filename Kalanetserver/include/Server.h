#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "DataManager.h"

class ClientHandler : public QObject {
    Q_OBJECT
public:
    explicit ClientHandler(QTcpSocket* socket, DataManager* dm, QObject* parent = nullptr);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void processCommand(const QString& cmd);
    void sendResponse(const QString& response);
    void sendError(const QString& msg);

    QTcpSocket* m_socket;
    DataManager* m_dataManager;
    QString m_buffer;
    User* m_currentUser; // authenticated user for this client
};

class Server : public QTcpServer {
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);
    bool start(quint16 port);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    DataManager* m_dataManager;
    QList<ClientHandler*> m_clients;
};

#endif // SERVER_H