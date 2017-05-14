#ifndef PROMISEHTTP_H
#define PROMISEHTTP_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QHash>
#include "promise.h"

class PromiseHttp : public QNetworkAccessManager
{
public:
    typedef CppPromise::Promise<QNetworkReply *> Promise;

    PromiseHttp(QObject *parent = nullptr);

    Promise get(const QNetworkRequest &request);

private slots:
    void OnFinished(QNetworkReply *pReply);

private:
    QHash<QNetworkReply *, Promise> m_mapReply;
};

#endif // PROMISEHTTP_H
