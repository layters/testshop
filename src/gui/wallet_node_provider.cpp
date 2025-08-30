#include "wallet_node_provider.hpp"

#include "../core/wallet/wallet.hpp" // WalletNetworkType, WalletNetworkPortMap
#include "../core/tools/string.hpp"  // string_tools::contains
#include "../core/tools/script.hpp"  // neroshop::Script::get_table_string
#include "../core/settings.hpp"      // lua_state
#include "settings_manager.hpp"      // SettingsManager

#include <QCoreApplication> // QCoreApplication::aboutToQuit
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMutexLocker>
#include <QUrl>

namespace neroshop {

//-----------------------------------------------------------------------------

WalletNodeProvider::WalletNodeProvider(QObject* parent) : QObject(parent) {
    m_networkManager.setParent(this);
    connect(&m_networkManager, &QNetworkAccessManager::finished,
        this, &WalletNodeProvider::handleNetworkReply);
        
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        if (m_updateTimer.isActive()) {
            m_updateTimer.stop();
            disconnect(&m_updateTimer, nullptr, this, nullptr);
        }
    });
    
    // Load last selected node from QSettings
    QString lastSelectedNode = SettingsManager::instance().getString("monero/daemon/last_selected_node");
    // Use QTimer::singleShot(0, ...) to defer signal emission until object fully constructed and event loop running
    QTimer::singleShot(0, this, [this, lastSelectedNode]() {
        setLastSelectedNode(lastSelectedNode); // signal emitted here
        if(!lastSelectedNode.isEmpty())
            std::cout << "Last selected node: " << lastSelectedNode.toStdString() << "\n";
    });
}

//-----------------------------------------------------------------------------

void WalletNodeProvider::startUpdates(int updateIntervalMs) {
    setLoadingNodes(true); // start loading
    
    WalletNetworkType networkType = Wallet::get_network_type();
    if(networkType == WalletNetworkType::Mainnet) { // ban nodes only when on mainnet
        fetchBannedNodes(); // once at startup
    }
    
    // Stop timer if its already on and disconnect it from updates
    if (m_updateTimer.isActive()) {
        m_updateTimer.stop();
        disconnect(&m_updateTimer, &QTimer::timeout, this, &WalletNodeProvider::fetchNodes);
    }

    // Periodically update node list
    m_updateTimer.setInterval(updateIntervalMs);
    connect(&m_updateTimer, &QTimer::timeout, this, &WalletNodeProvider::fetchNodes, Qt::QueuedConnection);
    m_updateTimer.start();

    fetchNodes(); // initial fetch
}

//-----------------------------------------------------------------------------

void WalletNodeProvider::handleNetworkReply(QNetworkReply* reply) {
    if (!reply) return;
    const QUrl url = reply->request().url();
    if (url.host().contains("monero-ban-list")) {
        onBannedNodesReply(reply);
    } else if (url.host().contains("monero.fail")) {
        onNodesReply(reply);
    } else {
        reply->deleteLater();
    }
}

//-----------------------------------------------------------------------------

WalletNodeProvider* WalletNodeProvider::instance() {
    static WalletNodeProvider inst;
    return &inst;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void WalletNodeProvider::setCoinName(const QString &coin) {
    QMutexLocker locker(&m_mutex);
    m_coinName = coin.toLower(); // make coin name lowercase
}

//-----------------------------------------------------------------------------

void WalletNodeProvider::setLoadingNodes(bool loading) {
    if (m_loadingNodes != loading) {
        m_loadingNodes = loading;
        emit loadingNodesChanged();
    }
}

//-----------------------------------------------------------------------------

void WalletNodeProvider::setLastSelectedNode(const QString &node) {
    if (m_lastSelectedNode != node) {
        m_lastSelectedNode = node;
        emit lastSelectedNodeChanged();
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QVariantList WalletNodeProvider::getNodes() const {
    QMutexLocker locker(&m_mutex);
    return m_nodes;
}

//-----------------------------------------------------------------------------

const QSet<QString>& WalletNodeProvider::getBannedNodes() const {
    QMutexLocker locker(&m_mutex);
    return m_bannedNodes;
}

//-----------------------------------------------------------------------------

QString WalletNodeProvider::getCoinName() const {
    QMutexLocker locker(&m_mutex);
    return m_coinName;
}

//-----------------------------------------------------------------------------

QString WalletNodeProvider::getLastSelectedNode() const {
    return m_lastSelectedNode;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void WalletNodeProvider::onBannedNodesReply(QNetworkReply* reply) {
        QByteArray data = reply->readAll();
        reply->deleteLater();
        processBannedNodesData(data);
        emit bannedNodesUpdated();
}

void WalletNodeProvider::onNodesReply(QNetworkReply* reply) {
        QByteArray data = reply->readAll();
        reply->deleteLater();
        processNodesData(data);
        setLoadingNodes(false); // loading done
        emit nodesUpdated();
}

//-----------------------------------------------------------------------------
    
void WalletNodeProvider::fetchBannedNodes() {
    const QUrl banUrl("https://raw.githubusercontent.com/Boog900/monero-ban-list/refs/heads/main/ban_list.txt");
    m_networkManager.get(QNetworkRequest(banUrl));
}

void WalletNodeProvider::fetchNodes() {
    const QUrl url("https://monero.fail/health.json");
    m_networkManager.get(QNetworkRequest(url));
}

//-----------------------------------------------------------------------------

void WalletNodeProvider::processBannedNodesData(const QByteArray& data) {
    QSet<QString> bannedNodes;
    // Split by lines and insert each trimmed entry
    for (const QByteArray& line : data.split('\n')) {
        QString addr = QString::fromUtf8(line).trimmed();
        // Skip empty or fully commented lines (e.g., lines starting with '#')
        if (addr.isEmpty() || addr.startsWith('#'))
            continue;
        
        // Removing trailing comment portion if any (e.g., after '#')    
        int commentIndex = addr.indexOf('#');
        if (commentIndex != -1)
            addr = addr.left(commentIndex).trimmed();
            
        bannedNodes.insert(addr);//std::cout << addr.toStdString() << std::endl;
    }
    
    QMutexLocker locker(&m_mutex);
    m_bannedNodes = std::move(bannedNodes);
}

void WalletNodeProvider::processNodesData(const QByteArray& data) {
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    // Use fallback nodes if we fail to get the nodes from the url
    auto defaultNodes = [this]() -> QVariantList {
        QVariantList nodeList;
        std::string networkTypeStr = Wallet::get_network_type_as_string();
        std::vector<std::string> nodeTable = neroshop::Script::get_table_string(neroshop::lua_state, m_coinName.toStdString() + ".nodes." + networkTypeStr); // Get monero nodes from settings.lua////std::cout << "lua_query: " << coin.toStdString() + ".nodes." + network_type << std::endl;
        for(auto strings : nodeTable) {
            nodeList << QString::fromStdString(strings);
        }
        return nodeList;
    };
    if (error.error != QJsonParseError::NoError) {
        QVariantList nodes = defaultNodes();
        {
            QMutexLocker locker(&m_mutex);
            m_nodes = std::move(nodes);
        } // <--- unlock mutex here
        return;
    }
    
    // Get wallet network ports
    WalletNetworkType networkType = Wallet::get_network_type();
    auto networkPorts = WalletNetworkPortMap[networkType];
    // Get banned wallet nodes
    QSet<QString> bannedNodes;
    {
        QMutexLocker locker(&m_mutex);
        bannedNodes = m_bannedNodes;
    } // <--- unlock mutex here
    // Get wallet nodes from the JSON
    QJsonObject rootObj = jsonDoc.object(); // {}
    QJsonObject coinObj = rootObj.value(m_coinName).toObject(); // "monero": {}, "wownero": {}
    QJsonObject clearnetObj = coinObj.value("clear").toObject(); // "clear": {}, "onion": {}, "web_compatible": {}
    QList<QVariantMap> nodeList; // intermediate list to hold node info maps
    // Loop through wallet nodes (clearnet)
    foreach(const QString& key, clearnetObj.keys()) {//for (const auto monero_nodes : clearnetObj) {
        QJsonObject nodeJsonObj = clearnetObj.value(key).toObject();//QJsonObject nodeJsonObj = monero_nodes.toObject();
        QVariantMap nodeObj; // Create an object for each row
        if(string_tools::contains_substring(key.toStdString(), networkPorts)) {
            QUrl url(key); // e.g. "http://123.45.67.89:18081" or "https://[2001:db8::1]:18081"
            if (url.isValid()) {
                QString ipAddress = url.host();  // Extracts the IP without scheme or port            
                if (bannedNodes.contains(ipAddress)) {
                    std::cout << "Banned: " << ipAddress.toStdString() << std::endl;
                    continue; // skip banned IPs
                }
            }
        
            nodeObj.insert("address", key);
            nodeObj.insert("available", nodeJsonObj.value("available").toBool());//std::cout << "available: " << nodeJsonObj.value("available").toBool() << "\n";
            nodeObj.insert("datetime_checked", nodeJsonObj.value("datetime_checked").toString());//std::cout << "datetime_checked: " << nodeJsonObj.value("datetime_checked").toString().toStdString() << "\n";
            nodeObj.insert("datetime_entered", nodeJsonObj.value("datetime_entered").toString());//std::cout << "datetime_entered: " << nodeJsonObj.value("datetime_entered").toString().toStdString() << "\n";
            nodeObj.insert("datetime_failed", nodeJsonObj.value("datetime_failed").toString());//std::cout << "datetime_failed: " << nodeJsonObj.value("datetime_failed").toString().toStdString() << "\n";
            nodeObj.insert("last_height", nodeJsonObj.value("last_height").toInt());//std::cout << "last_height: " << nodeJsonObj.value("last_height").toInt() << "\n";
            nodeList.append(nodeObj);
        }
    }
    
    // Sort nodeList by "available" key descending, after finishing the loop
    std::sort(nodeList.begin(), nodeList.end(), [](const QVariantMap &a, const QVariantMap &b) {
        bool aAvailable = a.value("available").toBool();
        bool bAvailable = b.value("available").toBool();
        // Sort available (true) nodes before unavailable (false) nodes
        return aAvailable > bAvailable;
    });
    
    // Now append sorted maps to the QVariantList to return
    QVariantList nodes;
    for (const QVariantMap &node : nodeList) {
        nodes.append(node);
    }
    
    {
        QMutexLocker locker(&m_mutex);
        m_nodes = std::move(nodes);
    } // <--- unlock mutex here
    
    // nodesUpdated() already emitted in onNodesReply() so don't emit twice
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool WalletNodeProvider::isLoadingNodes() const { return m_loadingNodes; }

//-----------------------------------------------------------------------------

} // namespace neroshop

