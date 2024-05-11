#pragma once

#ifndef PROXY_MANAGER_HPP_NEROSHOP
#define PROXY_MANAGER_HPP_NEROSHOP

#include <QObject>
#include <QNetworkAccessManager>
#include <QQmlNetworkAccessManagerFactory>
#include <QNetworkReply>
#include <QProcess>

namespace neroshop {
    
class ProxyManager : public QObject, public QQmlNetworkAccessManagerFactory {
    Q_OBJECT
    Q_PROPERTY(bool torEnabled READ isTorEnabled NOTIFY networkProxyChanged)
    Q_PROPERTY(QString torOutput READ getTorOutput NOTIFY torOutputChanged)
public:
    ProxyManager(QObject *parent = nullptr);
    ~ProxyManager();
    
    QNetworkAccessManager * create(QObject *parent) override;
    
    Q_INVOKABLE void useDefaultProxy();
    Q_INVOKABLE void useTorProxy();
    Q_INVOKABLE void useI2PProxy();
    
    Q_INVOKABLE static void downloadTor();
    Q_INVOKABLE void startTorDaemon();
    Q_INVOKABLE void stopTorDaemon();
    
    void setTorEnabled(bool torEnabled);
    
    QNetworkAccessManager * getNetworkClearnet() const;
    QNetworkAccessManager * getNetworkTor() const;
    QNetworkAccessManager * getNetworkI2P() const;
    QNetworkAccessManager * getNetwork() const;
    
    QString getTorOutput() const;

    Q_INVOKABLE static bool hasTor();
    Q_INVOKABLE static bool isTorRunning();
    Q_INVOKABLE bool isTorEnabled() const;
public slots:    
    QNetworkReply * getUrl(const QString& url);
    void onReplyFinished(QNetworkReply * reply);
signals:
    void networkProxyChanged();
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void torOutputChanged(const QString &output);
private:
    static void extractTar(const QString& fileName);
    QNetworkAccessManager * clearnetManager;
    QNetworkAccessManager * torManager;
    QNetworkAccessManager * i2pManager;
    bool m_torEnabled; // tor status
    QProcess *torProcess = nullptr;
    QString torOutput;
};

}
#endif
