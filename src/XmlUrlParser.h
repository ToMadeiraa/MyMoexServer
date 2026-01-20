#ifndef XMLURLPARSER_H
#define XMLURLPARSER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QHash>


class XmlUrlParser : public QObject {
    Q_OBJECT
public:
    explicit XmlUrlParser(QObject *parent = nullptr);
    void fetchXml(const QUrl &url);
    QString bigInsertString;
    QString LastSecurity_tmp;
    long long int LastTradeno_tmp;
    QHash<QString, ushort> SecID_Numbers;

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;

signals:
    void insertInDB_signal();
};

#endif // XMLURLPARSER_H
