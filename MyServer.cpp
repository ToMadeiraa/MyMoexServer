#include "MyServer.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{
    startServer();
}

void Server::startServer()
{
    //запустили сервер
    if (listen(QHostAddress::Any, 6666)) {
        qDebug() << "Server started on port 6666";
    } else {
        qDebug() << "Server could not start!";
    }

    //подключились к БД
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString homeDir = QDir::homePath();
    homeDir.append("/MyServer/MyMoexServer/MyDatabase/moex.db");
    db.setDatabaseName(homeDir);

    if (!db.open()) {
        qDebug() << "Ошибка подключения к базе данных:" << db.lastError().text();
    } else {
        qDebug() << "ПОДКЛЮЧЕНО";
    }


    requestQuery = new QSqlQuery(db);
    //создали таблицу, если не существует
    requestQuery->exec("CREATE TABLE IF NOT EXISTS moex ( TRADENO BIGINT UNIQUE, SECID VARCHAR(4), PRICE FLOAT, QUANTITY INTEGER, SYSTIME DATETIME, BUYSELL CHARACTER(1) );");
    requestQuery->first();

    //получили последний номер сделки
    requestQuery->exec("SELECT MAX(TRADENO) FROM moex;");
    requestQuery->first();
    LastTRADENO = requestQuery->value(0).toLongLong();
    LastTRADENOs["AFLT"] = LastTRADENO;
    qDebug() << "LAST TRADENO = " << LastTRADENO;

    //каждую секунду делаем парсинг
    timerSendRequest = new QTimer;
    connect(timerSendRequest, SIGNAL(timeout()), this, SLOT(makeParserRequests()));
    timerSendRequest->start(5000);

    //коннектим сигнал вставки хмл в БД
    connect(&parser, SIGNAL(insertInDB_signal()), this, SLOT(insertInDB_slot()));
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New connection received, descriptor:" << socketDescriptor;

    QThread *thread = new QThread(this);
    ConnectionHandler *handler = new ConnectionHandler(socketDescriptor);
    handler->requestQuery = this->requestQuery;
    handler->db = this->db;
    handler->moveToThread(thread);

    connect(thread, &QThread::started, handler, &ConnectionHandler::process);
    connect(handler, &ConnectionHandler::finished, thread, &QThread::quit);
    connect(handler, &ConnectionHandler::finished, handler, &ConnectionHandler::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
    threads.append(thread);
}

void Server::makeParserRequests()
{
    for (auto i = LastTRADENOs.cbegin(), end = LastTRADENOs.cend(); i != end; i++) {
        QString QUrl_str = "https://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/"; QUrl_str.append(i.key());
        QUrl_str.append("/trades.xml?TRADENO="); QUrl_str.append(QString::number(i.value()+1));
        QUrl_str.append("&limit=");
        QUrl_str.append(LIMITstr);
        //qDebug() << QUrl_str;
        parser.fetchXml(QUrl(QUrl_str));
    }
}

void Server::insertInDB_slot()
{
    //qDebug() << parser.bigInsertString;
    requestQuery->exec(parser.bigInsertString);
    requestQuery->first();
    LastTRADENOs[parser.LastSecurity_tmp] = parser.LastTradeno_tmp;
}


// https://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/AFLT/trades?start=2
//CREATE TABLE IF NOT EXISTS "moex" ( "TRADENO" BIGINT UNIQUE, "SECID" VARCHAR(4), "PRICE" FLOAT, "QUANTITY" INTEGER, "SYSTIME" DATETIME, "BUYSELL" CHARACTER(1) )
