#include "MyServer.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{
    startServer();
}

void Server::startServer()
{
    if (listen(QHostAddress::Any, 6666)) {
        qDebug() << "Server started on port 6666";
    } else {
        qDebug() << "Server could not start!";
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New connection received, descriptor:" << socketDescriptor;

    QThread *thread = new QThread(this);
    ConnectionHandler *handler = new ConnectionHandler(socketDescriptor);
    handler->moveToThread(thread);

    connect(thread, &QThread::started, handler, &ConnectionHandler::process);
    connect(handler, &ConnectionHandler::finished, thread, &QThread::quit);
    connect(handler, &ConnectionHandler::finished, handler, &ConnectionHandler::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
    threads.append(thread);
}

ConnectionHandler::ConnectionHandler(qintptr socketDescriptor, QObject *parent)
    : QObject(parent), socketDescriptor(socketDescriptor)
{
}

void ConnectionHandler::process()
{
    socket = new QTcpSocket();

    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "Error setting socket descriptor";
        emit finished();
        return;
    }

    qDebug() << "Client connected:" << socket->peerAddress().toString() << ":" << socket->peerPort();

    connect(socket, &QTcpSocket::readyRead, this, &ConnectionHandler::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ConnectionHandler::disconnected);
}

void ConnectionHandler::readyRead()
{
    QByteArray data = socket->readAll();
    qDebug() << "Received from client:" << data;

    socket->write(QString("Echo: " + data).toUtf8());
}

void ConnectionHandler::disconnected()
{
    qDebug() << "Client disconnected";
    socket->deleteLater();
    emit finished();
}
// https://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/AFLT/trades?start=2
