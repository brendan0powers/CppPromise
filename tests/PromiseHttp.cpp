#include "PromiseHttp.h"
#include <QNetworkReply>

PromiseHttp::PromiseHttp(QObject *parent)
    : QNetworkAccessManager(parent)
{
    connect(this, &PromiseHttp::finished,
            this, &PromiseHttp::OnFinished);


}

PromiseHttp::Promise PromiseHttp::get(const QNetworkRequest &request)
{
    Promise promise;
    QNetworkReply *pReply = QNetworkAccessManager::get(request);
    m_mapReply[pReply] = promise;
    return promise;
}

void PromiseHttp::OnFinished(QNetworkReply *pReply)
{
    if(!m_mapReply.contains(pReply))
        pReply->deleteLater();

    Promise promise = m_mapReply[pReply];
    if(pReply->error() == QNetworkReply::NoError)
        promise = CppPromise::resolve(pReply);
    else
        promise = CppPromise::reject<QNetworkReply*>(pReply);
}
