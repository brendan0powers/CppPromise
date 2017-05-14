#include <QCoreApplication>
#include <memory>
#include <functional>
#include <QDebug>
#include "Promise.h"
#include "PromiseHttp.h"
#include <QNetworkReply>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace CppPromise;

//Return an already resolved promise containing an int
Promise<int> testReturnInt()
{
    return resolve(123);
}

//Return an already resolved promise containing a float
Promise<float> testReturnFloat()
{
    return resolve(1.23f);
}

TEST_CASE("Basic Tests", "[Basic]")
{
    int argc = 0;
    char **argv = NULL;
    QCoreApplication a(argc, argv);

    testReturnInt().then([](int iTest) {
       REQUIRE(iTest == 123);
       return testReturnFloat();
    }).then([](float fTest) {
        REQUIRE(fTest == Approx(1.23));
        testReturnInt();
    }).then([]() {
        REQUIRE(true);
        return reject<void>((int)345);
    }).fail([](float err) {
        qDebug() << "Error float:" << err;
    }).fail([](float err) {
        qDebug() << "Error int2:" << err;
    }).fail([](QString err) {
        qDebug() << "Error QString:" << err;
    }).fail([](std::exception_ptr errPtr) {
        REQUIRE_THROWS_AS(std::rethrow_exception(errPtr), int);
        REQUIRE_THROWS_WITH(std::rethrow_exception(errPtr), "345");
    }).failAny([](){
        qDebug() << "Final Error";
        throw new QString("BOGUS");
    }).always([](){
        qDebug() << "POTATO";
    });

    PromiseHttp http;
    http.get(QNetworkRequest(QUrl("http://mfgcentral.908devices.office/api/boards2")))
            .then([](QNetworkReply *pReply){
       qDebug() << "Success:" << pReply->readAll();
    }).fail([](QNetworkReply *pReply){
        qDebug() << "Failed:" << pReply->error();
    }).failAny([](){
        qDebug() << "Something happened...";
    });
}
