#pragma once

#ifndef WALLET_NODE_PROVIDER_HPP_NEROSHOP
#define WALLET_NODE_PROVIDER_HPP_NEROSHOP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSet>
#include <QVariantList>
#include <QTimer>
#include <QMutex>
/*#include <QtConcurrent>
#include <QFutureWatcher>*/

namespace neroshop {

class WalletNodeProvider : public QObject { // provides a list of remote blockchain nodes
    Q_OBJECT
    Q_PROPERTY(QVariantList nodes READ getNodes NOTIFY nodesUpdated)
    Q_PROPERTY(QSet<QString> bannedNodes READ getBannedNodes NOTIFY bannedNodesUpdated)
    Q_PROPERTY(bool loadingNodes READ isLoadingNodes NOTIFY loadingNodesChanged)
    Q_PROPERTY(QString lastSelectedNode READ getLastSelectedNode NOTIFY lastSelectedNodeChanged)
public:
    static WalletNodeProvider* instance();
    
    Q_INVOKABLE void startUpdates(int updateIntervalMs = 3600000); // e.g., update every hour
    
    Q_INVOKABLE void setCoinName(const QString &coin);
    void setLoadingNodes(bool loading);
    void setLastSelectedNode(const QString &node);

    QVariantList getNodes() const;
    const QSet<QString>& getBannedNodes() const;
    QString getCoinName() const;
    QString getLastSelectedNode() const;
    
    bool isLoadingNodes() const;

signals:
    void nodesUpdated();
    void bannedNodesUpdated();
    void loadingNodesChanged();
    void lastSelectedNodeChanged();

private slots:
    void fetchNodes();
    void fetchBannedNodes();
    void handleNetworkReply(QNetworkReply* reply);
    void onNodesReply(QNetworkReply* reply);
    void onBannedNodesReply(QNetworkReply* reply);

private:
    explicit WalletNodeProvider(QObject *parent = nullptr);
    void processNodesData(const QByteArray &data);
    void processBannedNodesData(const QByteArray &data);
    QNetworkAccessManager m_networkManager;
    mutable QMutex m_mutex;
    QString m_coinName;
    QVariantList m_nodes;
    QSet<QString> m_bannedNodes;
    bool m_loadingNodes = false;
    QString m_lastSelectedNode;
    QTimer m_updateTimer;
    ////QFutureWatcher<void> m_futureWatcher;
};

}

#endif // WALLET_NODE_PROVIDER_HPP_NEROSHOP

