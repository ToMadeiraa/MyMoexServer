#include <QCoreApplication>

#include "MyServer.h"
#include "XmlUrlParser.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
    XmlUrlParser parser;
    parser.fetchXml(QUrl("https://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/AFLT/trades.xml?TRADENO=13041198505&limit=10"));

    return a.exec();
}



