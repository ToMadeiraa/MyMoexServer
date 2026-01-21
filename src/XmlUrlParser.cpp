#include "XmlUrlParser.h"

XmlUrlParser::XmlUrlParser(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &XmlUrlParser::replyFinished);
}

void XmlUrlParser::fetchXml(const QUrl &url) {
    QNetworkRequest request(url);
    manager->get(request);
}

void XmlUrlParser::replyFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    bigInsertString = "INSERT INTO ";

    bool flag = false;
    QXmlStreamReader xml(reply->readAll());
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name().toString() == "row" && xml.attributes().size() == 15) {

            long long int TRADENO_tmp = xml.attributes().at(0).value().toLongLong();

            if (flag == false)
            {
                QString SECID_tmp = xml.attributes().at(3).value().toString();
                QString companyName = SECID_tmp + QString("_server (TRADENO, PRICE, QUANTITY, SYSTIME, BUYSELL) VALUES ");
                bigInsertString.append(companyName);
                LastSecurity_tmp = SECID_tmp;
                flag = true;
            }

            double PRICE_tmp = xml.attributes().at(4).value().toDouble();
            int QUANTITY_tmp = xml.attributes().at(5).value().toInt();
            QString SYSTIME_tmp = xml.attributes().at(9).value().toString();
            QString BUYSELL_tmp = xml.attributes().at(10).value().toString();
            char BUYSELL_tmp_bool = (BUYSELL_tmp == "B") ? '1' : '0';

            QString valuesInsertString = "(";
            valuesInsertString.append(QString::number(TRADENO_tmp)); valuesInsertString.append(", ");
            valuesInsertString.append(QString::number(PRICE_tmp)); valuesInsertString.append(", ");
            valuesInsertString.append(QString::number(QUANTITY_tmp)); valuesInsertString.append(", '");
            valuesInsertString.append(SYSTIME_tmp); valuesInsertString.append("', ");
            valuesInsertString.append(BUYSELL_tmp_bool); valuesInsertString.append("),");
            bigInsertString.append(valuesInsertString);
            LastTradeno_tmp = TRADENO_tmp;
        }
    }
    bigInsertString.chop(1);
    bigInsertString.append(";");

    emit (insertInDB_signal());
    reply->deleteLater();
}
