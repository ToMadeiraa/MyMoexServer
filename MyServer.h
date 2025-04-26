#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QVector>
#include <QDebug>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void startServer();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QVector<QThread*> threads;
};

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHandler(qintptr socketDescriptor, QObject *parent = nullptr);

signals:
    void finished();

public slots:
    void process();

private slots:
    void readyRead();
    void disconnected();

private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
};

#endif // MYSERVER_H
