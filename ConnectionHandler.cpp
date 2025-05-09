#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(qintptr socketDescriptor_, QObject *parent)
    : QObject(parent), socketDescriptor(socketDescriptor_)
{
    SecID_Numbers["AFLT"] = 1;
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
    timerSendData->start(500);

    connect(socket, &QTcpSocket::readyRead, this, &ConnectionHandler::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ConnectionHandler::disconnected);
}

void ConnectionHandler::readyRead()
{
    QByteArray data = socket->readAll();
    QDataStream ds(&data, QIODevice::ReadOnly);
    long long int word = 0;

    while (!ds.atEnd()) {
        ds >> word;
        ushort secid_tmp = word & 0x3FF; word = word >> 9;
        long long int lastTradeno_tmp = word;
        clientLastTradeno[secid_tmp] = lastTradeno_tmp;
    }
}

void ConnectionHandler::disconnected()
{
    qDebug() << "Client disconnected";
    socket->deleteLater();
    emit finished();
}

void ConnectionHandler::sendData_slot()
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);

    ushort secid_tmp = 0;
    long long int tradeno_tmp = 0;
    long long int tradeno_tmp_for_hash = 0;
    float price_tmp = 0;
    int quantity_tmp = 0;
    QString systime_tmp;
    ushort buysell_tmp;

    QDateTime dt_2000; dt_2000.setDate(QDate(2000,1,1)); dt_2000.setTime(QTime(0,0,0));

    //получаем из бд 10 записей определенного secid с определенным tradeno
    // и отправляем клиенту, заодно записывая какие последние tradeno мы отправили
    for (auto i = clientLastTradeno.cbegin(), end = clientLastTradeno.cend(); i != end; i++) {

        secid_tmp = i.key();
        tradeno_tmp = i.value();

        qDebug() << "SECID = " << secid_tmp;
        qDebug() << "TRADENO = " << tradeno_tmp;

        QString req = "SELECT TRADENO, SECID, PRICE, QUANTITY, SYSTIME, BUYSELL FROM moex";
        req.append(" WHERE SECID = "); req.append(QString::number(secid_tmp));
        req.append(" AND TRADENO > "); req.append(QString::number(tradeno_tmp));
        req.append(" LIMIT " + QString::number(DATA_LIMIT) + ";");

        requestQuery->exec(req);
        requestQuery->first();

        //пока не дойдем до последней записи
        while (requestQuery->next()) {

            tradeno_tmp = requestQuery->value(0).toLongLong(); tradeno_tmp_for_hash = tradeno_tmp;
            secid_tmp = requestQuery->value(1).toUInt();
            price_tmp = requestQuery->value(2).toDouble();
            quantity_tmp = requestQuery->value(3).toUInt();
            systime_tmp = requestQuery->value(4).toString();
            buysell_tmp = requestQuery->value(5).toUInt();

            tradeno_tmp = (tradeno_tmp << 10);
            tradeno_tmp = tradeno_tmp | secid_tmp;

            tradeno_tmp = (tradeno_tmp << 1);
            tradeno_tmp = tradeno_tmp | buysell_tmp;

            ds << tradeno_tmp;
            ds << price_tmp;
            ds << quantity_tmp;

            QDateTime dt_now = QDateTime::fromString(systime_tmp, "yyyy-MM-dd hh:mm:ss");
            uint secondsFrom2000 = dt_2000.secsTo(dt_now);
            ds << secondsFrom2000;
        }

        socket->write(ba);
        clientLastTradeno[secid_tmp] = tradeno_tmp_for_hash;
    }
}
