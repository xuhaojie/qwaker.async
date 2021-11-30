#ifndef __WAKER_H__
#define __WAKER_H__

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkCookie>

const QString LOGIN_URL ="https://192.168.5.1:8443/login.cgi";

class Waker : public QObject {
    Q_OBJECT
public:

    explicit Waker(const QString&url, const QString& userName, const QString& password);
    virtual ~Waker();

protected:
    void dumpCookies();
    QSslConfiguration ssl_config;
    QNetworkAccessManager* manager;
    QString base_url;
    QString auth;
    QString token;

signals:
    void signalLogin();
    void signalExecuteCommand(const QString& cmd);
    void signalLogout();

    void signalLoginResult(bool result);
    void signalExecuteCommandResult(bool result);
    void signalLogoutResult(bool result);

private:
    void doLogin();
    void doExecuteCommand(const QString& cmd);
    void doLogout();

    void loginReplyFinished();
    void executeCommandReplyFinished();
    void logoutReplyFinished();
};


#endif // __WAKER_H__
