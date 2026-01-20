#include "XmlUrlParser.h"

XmlUrlParser::XmlUrlParser(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &XmlUrlParser::replyFinished);

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



//    bigInsertString = "INSERT INTO moex (TRADENO, SECID, PRICE, QUANTITY, SYSTIME, BUYSELL) VALUES ";
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

    qDebug() << bigInsertString;
    emit (insertInDB_signal());
    reply->deleteLater();





//    bigInsertString = "INSERT INTO moex (TRADENO, SECID, PRICE, QUANTITY, SYSTIME, BUYSELL) VALUES ";

//    QXmlStreamReader xml(reply->readAll());
//    while (!xml.atEnd() && !xml.hasError()) {
//        xml.readNext();

//        if (xml.name().toString() == "row" && xml.attributes().size() == 15) {

//            long long int TRADENO_tmp = xml.attributes().at(0).value().toLongLong();
//            QString SECID_tmp = xml.attributes().at(3).value().toString();
//            double PRICE_tmp = xml.attributes().at(4).value().toDouble();
//            int QUANTITY_tmp = xml.attributes().at(5).value().toInt();
//            QString SYSTIME_tmp = xml.attributes().at(9).value().toString();
//            QString BUYSELL_tmp = xml.attributes().at(10).value().toString();
//            char BUYSELL_tmp_bool = (BUYSELL_tmp == "B") ? '1' : '0';

//            QString valuesInsertString = "(";
//            valuesInsertString.append(QString::number(TRADENO_tmp)); valuesInsertString.append(", ");
//            valuesInsertString.append(QString::number(SecID_Numbers[SECID_tmp])); valuesInsertString.append(", ");
//            valuesInsertString.append(QString::number(PRICE_tmp)); valuesInsertString.append(", ");
//            valuesInsertString.append(QString::number(QUANTITY_tmp)); valuesInsertString.append(", '");
//            valuesInsertString.append(SYSTIME_tmp); valuesInsertString.append("', ");
//            valuesInsertString.append(BUYSELL_tmp_bool); valuesInsertString.append("),");
//            bigInsertString.append(valuesInsertString);
//            LastTradeno_tmp = TRADENO_tmp;
//            LastSecurity_tmp = SECID_tmp;

//        }
//    }
//    bigInsertString.chop(1);
//    bigInsertString.append(";");

//    emit (insertInDB_signal());
//    reply->deleteLater();
}
