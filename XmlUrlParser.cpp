#include "XmlUrlParser.h"

XmlUrlParser::XmlUrlParser(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &XmlUrlParser::replyFinished);
}

void XmlUrlParser::fetchXml(const QUrl &url) {
    QNetworkRequest request(url);
    manager->get(request);
}

void XmlUrlParser::replyFinished(QNetworkReply *reply) {
    // if (reply->error() != QNetworkReply::NoError) {
    //     qDebug() << "Error:" << reply->errorString();
    //     reply->deleteLater();
    //     return;
    // }


    Security sec_tmp;
    bigInsertString = "INSERT INTO moex (TRADENO, SECID, PRICE, QUANTITY, SYSTIME, BUYSELL) VALUES ";

    QXmlStreamReader xml(reply->readAll());
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name().toString() == "row" && xml.attributes().size() == 15) {

            // qDebug() << xml.attributes().at(0).value(); //TRADENO
            // qDebug() << xml.attributes().at(3).value(); //SECID
            // qDebug() << xml.attributes().at(4).value(); //PRICE
            // qDebug() << xml.attributes().at(5).value(); //QUANTITY
            // qDebug() << xml.attributes().at(9).value(); //SYSTIME
            // qDebug() << xml.attributes().at(10).value(); //BUYSELL

            long long int TRADENO_tmp = xml.attributes().at(0).value().toLongLong();
            QString SECID_tmp = xml.attributes().at(3).value().toString();
            double PRICE_tmp = xml.attributes().at(4).value().toDouble();
            int QUANTITY_tmp = xml.attributes().at(5).value().toInt();
            QString SYSTIME_tmp = xml.attributes().at(9).value().toString();
            QString BUYSELL_tmp = xml.attributes().at(10).value().toString();

            QString valuesInsertString = "(";
            valuesInsertString.append(QString::number(TRADENO_tmp)); valuesInsertString.append(", '");
            valuesInsertString.append(SECID_tmp); valuesInsertString.append("', ");
            valuesInsertString.append(QString::number(PRICE_tmp)); valuesInsertString.append(", ");
            valuesInsertString.append(QString::number(QUANTITY_tmp)); valuesInsertString.append(", '");
            valuesInsertString.append(SYSTIME_tmp); valuesInsertString.append("', '");
            valuesInsertString.append(BUYSELL_tmp); valuesInsertString.append("'),");
            bigInsertString.append(valuesInsertString);
            LastTradeno_tmp = TRADENO_tmp;
            LastSecurity_tmp = SECID_tmp;

        }
    }
    bigInsertString.chop(1);
    bigInsertString.append(";");

    emit (insertInDB_signal());
    reply->deleteLater();
    //qDebug() << "=====================================================";
}
