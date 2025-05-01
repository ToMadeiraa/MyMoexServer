#ifndef XMLURLPARSER_H
#define XMLURLPARSER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QHash>

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
    explicit XmlUrlParser(QObject *parent = nullptr);
    void fetchXml(const QUrl &url);
    QString bigInsertString;
    QString LastSecurity_tmp;
    long long int LastTradeno_tmp;

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;

signals:
    void insertInDB_signal();
};

#endif // XMLURLPARSER_H
