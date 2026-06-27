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

    //заполняем таблицу secid/ushort, чтобы было дешевле отправлять по сети
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


    //подключились к БД
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName("db_server");
    db.setUserName("postgres");
    db.setPassword("12345678");

    if (!db.open()) {
        qDebug() << "Ошибка подключения к базе данных:" << db.lastError().text();
    } else {
        qDebug() << "ПОДКЛЮЧЕНО";
    }

    requestQuery = new QSqlQuery(db);

    //создаем таблицы
    for (const auto &key : SecID_Numbers.keys())
    {
        QString companyName = QString(key) + QString("_server");
        qDebug() << "Creating " << companyName;
        requestQuery->exec("CREATE TABLE IF NOT EXISTS " + companyName + " ( TRADENO BIGINT UNIQUE, PRICE FLOAT, QUANTITY INTEGER, SYSTIME TIMESTAMP, BUYSELL SMALLINT );");
        requestQuery->first();
    }

    //получили последние номера сделки
    for (const auto &key : SecID_Numbers.keys())
    {
        QString companyName = QString(key) + QString("_server");
        QString req = "SELECT MAX(TRADENO) FROM "+ companyName;
        requestQuery->exec(req);
        requestQuery->first();
        LastTRADENOs[key] = requestQuery->value(0).toLongLong();
        qDebug() << "Last trade number of " << companyName << " == " << LastTRADENOs[key];
    }

    //каждые 10c делаем парсинг
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
    //handler->db = &this->db;
    handler->mtx = &this->mtx;
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
    // currentTradeNosVector.clear();
    // for (const auto &key : LastTRADENOs.keys())
    //     currentTradeNosVector.push_back(LastTRADENOs[key]);

    // bool isAllLoaded = false;
    // for (int i = 0; i < lastTradeNosVector.size(); ++i)
    // {
    //     if (lastTradeNosVector[i] != currentTradeNosVector[i])
    //     {
    //         isAllLoaded = false;
    //         break;
    //     }
    //     isAllLoaded = true;
    // }

    // if (isAllLoaded == true)
    // {
    //     qDebug() << "All data is loaded!";
    //     std::terminate();
    // }
    // else
    // {
    //     lastTradeNosVector = currentTradeNosVector;
    // }

    for (const auto &key : LastTRADENOs.keys())
    {
        QString QUrl_str = QString("https://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/")
                         + QString(key)
                         + QString("/trades.xml?TRADENO=")
                         + QString::number(LastTRADENOs[key]);

        qDebug() << key <<  " LastTRADENO = " << LastTRADENOs[key];

        parser.fetchXml(QUrl(QUrl_str));
        QThread::msleep(100);
    }
}

void Server::insertInDB_slot()
{
    //qDebug() << parser.bigInsertString;
    mtx.lock();

    requestQuery->exec(parser.bigInsertString);
    requestQuery->first();
    LastTRADENOs[parser.LastSecurity_tmp] = parser.LastTradeno_tmp+1;

    mtx.unlock();
}


// https://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/AFLT/trades?start=2
//CREATE TABLE IF NOT EXISTS "moex" ( "TRADENO" BIGINT UNIQUE, "SECID" VARCHAR(4), "PRICE" FLOAT, "QUANTITY" INTEGER, "SYSTIME" DATETIME, "BUYSELL" CHARACTER(1) )
