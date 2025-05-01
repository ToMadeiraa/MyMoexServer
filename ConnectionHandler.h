#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QVector>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDataStream>

#include "DataPacket.h"

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHandler(qintptr socketDescriptor, QObject *parent = nullptr);
    QTimer *timerSendData;
    DataPacket data;
    QSqlQuery* requestQuery;
    QSqlDatabase db;
signals:
    void finished();

public slots:
    void process();



private slots:
    void readyRead();
    void disconnected();
    void sendData_slot();

private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
};

#endif // CONNECTIONHANDLER_H
