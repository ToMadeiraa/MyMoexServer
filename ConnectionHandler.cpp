#include "ConnectionHandler.h"


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

    timerSendData = new QTimer;
    connect(timerSendData, SIGNAL(timeout()), this, SLOT(sendData_slot()));
    timerSendData->start(1000);

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

void ConnectionHandler::sendData_slot()
{

    // QByteArray ba;
    // QDataStream ds(ba);
    //socket->write();
    requestQuery->exec("SELECT MAX(TRADENO) FROM moex;");
    requestQuery->first();
    qDebug() << requestQuery->value(0).toLongLong();
}
