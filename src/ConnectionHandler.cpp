#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(qintptr socketDescriptor_, QObject *parent)
    : QObject(parent), socketDescriptor(socketDescriptor_)
{
    SecID_Numbers["GAZP"]   = 1;
    SecID_Numbers["SBER"]   = 2;
    SecID_Numbers["LKOH"]   = 3;
    SecID_Numbers["T"]      = 4;
    SecID_Numbers["PLZL"]   = 5;
    SecID_Numbers["RUAL"]   = 6;
    SecID_Numbers["MGNT"]   = 7;
    SecID_Numbers["NVTK"]   = 8;
    SecID_Numbers["GMKN"]   = 9;
    SecID_Numbers["VTBR"]   = 10;
    SecID_Numbers["AFLT"]   = 11;
    SecID_Numbers["MOEX"]   = 12;
    SecID_Numbers["NLMK"]   = 13;
    SecID_Numbers["TRNFP"]  = 14;
    SecID_Numbers["SELG"]   = 15;
    SecID_Numbers["SMLT"]   = 16;
    SecID_Numbers["SNGSP"]  = 17;
    SecID_Numbers["ROSN"]   = 18;
    SecID_Numbers["MTSS"]   = 19;
    SecID_Numbers["AFKS"]   = 20;
    SecID_Numbers["VKCO"]   = 21;
    SecID_Numbers["EUTR"]   = 22;
    SecID_Numbers["CHMF"]   = 23;
    SecID_Numbers["ELMT"]   = 24;
    SecID_Numbers["ALRS"]   = 25;
    SecID_Numbers["IRKT"]   = 26;
    SecID_Numbers["POSI"]   = 27;
    SecID_Numbers["UNAC"]   = 28;
    SecID_Numbers["RAGR"]   = 29;
    SecID_Numbers["MBNK"]   = 30;
    SecID_Numbers["SGZH"]   = 31;
    SecID_Numbers["KMAZ"]   = 32;
    SecID_Numbers["MVID"]   = 33;
    SecID_Numbers["TRMK"]   = 34;
    SecID_Numbers["RNFT"]   = 35;
    SecID_Numbers["FLOT"]   = 36;
    SecID_Numbers["YDEX"]   = 37;
    SecID_Numbers["ASTR"]   = 38;
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
    QDataStream ds(&data, QIODevice::ReadOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    long long int word = 0;

    while (!ds.atEnd())
    {
        ds >> word;
        ushort secid_tmp = word & 0x3FF; word = word >> 10;
        long long int lastTradeno_tmp = word;
        clientLastTradeno[secid_tmp] = lastTradeno_tmp;
    }


    sendData_slot();
}

void ConnectionHandler::disconnected()
{
    qDebug() << "Client disconnected";
    socket->deleteLater();
    emit finished();
}

void ConnectionHandler::sendData_slot()
{
    mtx->lock();

    ushort secid_tmp = 0;
    long long int tradeno_tmp = 0;
    float price_tmp = 0;
    int quantity_tmp = 0;
    QString systime_tmp;
    ushort buysell_tmp;

    QDateTime dt_2000; dt_2000.setDate(QDate(2000,1,1)); dt_2000.setTime(QTime(0,0,0));

    //получаем из бд DATA_LIMIT записей определенного secid с определенным tradeno
    // и отправляем клиенту, заодно записывая какие последние tradeno мы отправили
    for (auto i = clientLastTradeno.cbegin(), end = clientLastTradeno.cend(); i != end; i++)
    {
        QByteArray ba;
        QDataStream ds(&ba, QIODevice::WriteOnly);
        ds.setByteOrder(QDataStream::BigEndian);
        ds.setFloatingPointPrecision(QDataStream::SinglePrecision);

        secid_tmp = i.key();
        tradeno_tmp = i.value();

        QString companyName;
        for (const auto &key : SecID_Numbers.keys())
        {
            if (SecID_Numbers[key] == secid_tmp)
            {
                companyName = QString(key) + QString("_server");
                break;
            }
        }
        QString maxTradeno = "SELECT MAX(TRADENO) FROM " + companyName;
        requestQuery->exec(maxTradeno);
        requestQuery->first();
        long long int maxTradeNumber = requestQuery->value(0).toLongLong();

        while (maxTradeNumber > tradeno_tmp)
        {
            QString req = "SELECT TRADENO, PRICE, QUANTITY, SYSTIME, BUYSELL FROM " + companyName;
            req.append(" WHERE TRADENO > "); req.append(QString::number(tradeno_tmp));
            req.append(";");

            qDebug() << req;

            requestQuery->exec(req);
            requestQuery->first();

            //пока не дойдем до последней записи
            do
            {
                tradeno_tmp = requestQuery->value(0).toLongLong();
                price_tmp = requestQuery->value(1).toDouble();
                quantity_tmp = requestQuery->value(2).toUInt();
                systime_tmp = requestQuery->value(3).toString();
                buysell_tmp = requestQuery->value(4).toUInt();

                long long int word = tradeno_tmp;

                word = (word << 10);
                word = word | secid_tmp;

                word = (word << 1);
                word = word | buysell_tmp;

                ds << word;
                ds << price_tmp;
                ds << quantity_tmp;

                QDateTime dt_now = QDateTime::fromString(systime_tmp, "yyyy-MM-ddTHH:mm:ss.zzz");
                uint secondsFrom2000 = dt_2000.secsTo(dt_now);
                ds << secondsFrom2000;
            } while (requestQuery->next());
        }
        clientLastTradeno[secid_tmp] = tradeno_tmp;

        int sizeOfCompanyPack = ba.size();
        ba.prepend((const char*)&sizeOfCompanyPack, sizeof(sizeOfCompanyPack));

        socket->write(ba);
        socket->flush();
        QThread::msleep(50);
    }

    mtx->unlock();
}
