#ifndef MYSERVER_H
#define MYSERVER_H

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

#include <QStandardPaths>
#include <QDir>

#include "XmlUrlParser.h"
#include "ConnectionHandler.h"

#define LIMIT 1;
#define LIMITstr "1"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void startServer();

    XmlUrlParser parser;
    QHash<QString, long long int> LastTRADENOs;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QVector<QThread*> threads;
    QSqlDatabase db;
    QSqlQuery* requestQuery;
    QTimer *timerSendRequest;
    long long int LastTRADENO;

public slots:
    void makeParserRequests();
    void insertInDB_slot();

};


#endif // MYSERVER_H
