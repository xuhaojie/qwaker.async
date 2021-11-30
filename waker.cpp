#include "waker.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkProxy>
#include <QUrlQuery>
#include <QCoreApplication>

const char* CONTENT_TYPE = "application/x-www-form-urlencoded";

Waker::Waker(const QString&url, const QString& userName, const QString& password): base_url(url) {
    QString auth = userName;
    auth.append(":");
    auth.append(password);


    QByteArray text = auth.toLocal8Bit();

    this->auth = text.toBase64();

    this->manager = new QNetworkAccessManager(this);
//    cookie_jar = new QNetworkCookieJar(manager);
//    manager->setCookieJar(cookie_jar);
    /*
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(8888);
    */
//    manager->setProxy(proxy);
//    proxy.setUser("username");
//    proxy.setPassword("password");
//    QNetworkProxy::setApplicationProxy(proxy);
    // SSL认证
    ssl_config = QSslConfiguration::defaultConfiguration();
    ssl_config.setPeerVerifyMode(QSslSocket::VerifyNone);
    ssl_config.setProtocol(QSsl::TlsV1_2);

    connect(this, &Waker::signalLogin, this, &Waker::doLogin);
    connect(this, &Waker::signalExecuteCommand, this, &Waker::doExecuteCommand);
    connect(this, &Waker::signalLogout, this, &Waker::doLogout);
}

// 结束请求
Waker::~Waker() {
    delete manager;
}

void Waker::doLogin() {
    QUrlQuery query;
    query.addQueryItem("group_id","");
    query.addQueryItem("action_mode","");
    query.addQueryItem("action_script","");
    query.addQueryItem("action_wait","5");
    query.addQueryItem("login_authorization",this->auth);
    query.addQueryItem("next_page","Main_Login.asp");
    query.addQueryItem("current_page","index.asp");

    QByteArray dataArray = query.toString().toLocal8Bit();

    // 设置消息头
    QNetworkRequest request;
    request.setUrl(QUrl(this->base_url + "/login.cgi"));
    request.setSslConfiguration(ssl_config);
    request.setHeader(QNetworkRequest::ContentTypeHeader, CONTENT_TYPE);
    request.setRawHeader("Referer", QByteArray((this->base_url + QString("/Main_Login.asp")).toUtf8()));

    // 开始请求
    QNetworkReply *reply = manager->post(request, dataArray);
    connect(reply, &QNetworkReply::finished, this, &Waker::loginReplyFinished);
}

void Waker::doExecuteCommand(const QString& cmd) {

    QUrlQuery query;
    query.addQueryItem("group_id","");
    query.addQueryItem("action_mode","+Refresh+");
    query.addQueryItem("action_script","");
    query.addQueryItem("action_wait","");
    query.addQueryItem("next_page","index.asp");
    query.addQueryItem("current_page","Main_WOL_Content.asp");
    query.addQueryItem("firmver","3.0.0.4");
    query.addQueryItem("first_time","");
    query.addQueryItem("modified","0");
    query.addQueryItem("preferred_lang","CN");
    query.addQueryItem("SystemCmd",cmd);

    QByteArray dataArray = query.toString().toLocal8Bit();

    // 设置消息头
    QNetworkRequest request;
    request.setSslConfiguration(ssl_config);
    request.setHeader(QNetworkRequest::ContentTypeHeader, CONTENT_TYPE);
    QByteArray referer(this->base_url.toUtf8());// + "/Main_WOL_Content.asp");
    request.setRawHeader("Referer", referer);
    request.setUrl(QUrl(this->base_url + "/apply.cgi"));
    manager->cookieJar()->setProperty("manager->cookieJar()",this->token);

    // 开始请求
    QNetworkReply *reply = this->manager->post(request, dataArray);
    connect(reply, &QNetworkReply::finished, this, &Waker::executeCommandReplyFinished);
}

void Waker::doLogout()
{
    // 设置消息头
    QNetworkRequest request;
    request.setSslConfiguration(ssl_config);

    request.setUrl(QUrl(this->base_url + "/Logout.asp"));

    // 开始请求
    QNetworkReply *reply = this->manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &Waker::logoutReplyFinished);

}


void Waker::dumpCookies() {
    typedef QList<QNetworkCookie> CookieList;
    CookieList cookies = manager->cookieJar()->cookiesForUrl(this->base_url);
    CookieList::iterator it = cookies.begin();

    while(it != cookies.end()) {
        qDebug() << *it << Qt::endl;
        it++;
    }
}

// 登录响应结束
void Waker::loginReplyFinished() {
    QNetworkReply* reply = (QNetworkReply*)sender();

    QByteArray bytes = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    bool result = false;
    if (reply->error() || 200 != statusCode) {
        result = false;
    } else {
        QList<QNetworkCookie> cookies = reply->header(QNetworkRequest::SetCookieHeader).value<QList<QNetworkCookie>>();
        if(cookies.size() > 0){
            QString token = QString::fromLocal8Bit(cookies[0].value());
            this->token = token;
            result = true;
        }
    }

    reply->deleteLater();
    emit signalLoginResult(result);
}

// 执行命令响应结束
void Waker::executeCommandReplyFinished() {
    QNetworkReply* reply = (QNetworkReply*)sender();
    QByteArray bytes = reply->readAll();
    bool result = true;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() || 200 != statusCode) {
        result = false;
    }
    reply->deleteLater();
    emit signalExecuteCommandResult(result);
}

// 退出响应结束
void Waker::logoutReplyFinished() {
    QNetworkReply* reply = (QNetworkReply*)sender();
    QByteArray bytes = reply->readAll();
    bool result = true;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() || 200 != statusCode) {
        result = false;
    }
    reply->deleteLater();
    emit signalLogoutResult(true);
}

