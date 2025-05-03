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
#include <QDateTime>

#include <QDataStream>
#include <QHash>

#define DATA_LIMIT 10

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHandler(qintptr socketDescriptor, QObject *parent = nullptr);
    QTimer *timerSendData;
    QSqlQuery* requestQuery;
    QSqlDatabase db;

    QHash<QString, ushort> SecID_Numbers; //расшифровка secid по id
    QHash<ushort, long long int> clientLastTradeno;


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
