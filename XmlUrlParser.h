#ifndef XMLURLPARSER_H
#define XMLURLPARSER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>

struct Security {
    quint64 Tradeno;
    QString SecID;
    double Price;
    quint32 Quantity;
    QDateTime Systime;
    QChar BuySell;
};


class XmlUrlParser : public QObject {
    Q_OBJECT
public:
    explicit XmlUrlParser(QObject *parent = nullptr) : QObject(parent) {
        manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished,
                this, &XmlUrlParser::replyFinished);
    }

    void fetchXml(const QUrl &url) {
        QNetworkRequest request(url);
        manager->get(request);
    }

private slots:
    void replyFinished(QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error:" << reply->errorString();
            reply->deleteLater();
            return;
        }


        QXmlStreamReader xml(reply->readAll());
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();

            if (xml.isStartElement() && xml.name().toString() == "row") {
                qDebug() << "Start element:" << xml.name();

                // Access attributes
                for (const auto &attr : xml.attributes()) {
                    //if (attr.name().toString() == "TRADENO") {
                    qDebug() << attr.name() << "=" << attr.value();
                    //}
                }

                // Read element text if needed
                if (xml.name().toString() == "SomeElement") {
                    QString text = xml.readElementText();
                    qDebug() << "Element text:" << text;
                }
            }
        }

        if (xml.hasError()) {
            qDebug() << "XML Error:" << xml.errorString();
        }

        reply->deleteLater();
        qDebug() << "XML parsing completed";
    }

private:
    QNetworkAccessManager *manager;
};

#endif // XMLURLPARSER_H
