#include "proxy_manager.hpp"

#include <QFile>
#include <QNetworkDiskCache>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QNetworkProxyQuery>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

#include "../neroshop_config.hpp"
#include "../core/tools/filesystem.hpp"
#include "../core/tools/device.hpp"

neroshop::ProxyManager::ProxyManager(QObject* parent) : QObject(parent), m_externalProcess(false), m_torEnabled(false) {
    torManager = new QNetworkAccessManager(this);
    i2pManager = new QNetworkAccessManager(this);
    clearnetManager = new QNetworkAccessManager(this);
    
    // Connect finished signal to onReplyFinished slot
    connect(torManager, &QNetworkAccessManager::finished, this, &ProxyManager::onReplyFinished);
    connect(i2pManager, &QNetworkAccessManager::finished, this, &ProxyManager::onReplyFinished);
    connect(clearnetManager, &QNetworkAccessManager::finished, this, &ProxyManager::onReplyFinished);
    
    QNetworkProxy torProxy;
    torProxy.setType(QNetworkProxy::Socks5Proxy);
    torProxy.setHostName("127.0.0.1"); // Tor SOCKS5 proxy address
    torProxy.setPort(9050); // Tor SOCKS5 proxy port
    torManager->setProxy(torProxy);
    
    QNetworkProxy i2pProxy;
    i2pProxy.setType(QNetworkProxy::Socks5Proxy);
    i2pProxy.setHostName("127.0.0.1");
    i2pProxy.setPort(4447); // 4448?
    i2pManager->setProxy(i2pProxy);
}
    
neroshop::ProxyManager::~ProxyManager() {}

QNetworkAccessManager * neroshop::ProxyManager::create(QObject *parent) {
    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(parent);
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(parent);
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    diskCache->setCacheDirectory(cacheDir);
    networkAccessManager->setCache(diskCache);
    
    if(isTorEnabled() && isTorRunning()) {
        QNetworkProxy torProxy;
        torProxy.setType(QNetworkProxy::Socks5Proxy);
        torProxy.setHostName("127.0.0.1");
        torProxy.setPort(9050);
        networkAccessManager->setProxy(torProxy);
        // Standard QML components like Image or XmlHttpRequest can now make network requests over Tor
    }
    
    return networkAccessManager;
}

void neroshop::ProxyManager::useDefaultProxy() {
    QNetworkProxy::setApplicationProxy(QNetworkProxy());
    
    setTorEnabled(false);
}

void neroshop::ProxyManager::useTorProxy() {
    QNetworkProxy torProxy;
    torProxy.setType(QNetworkProxy::Socks5Proxy);
    torProxy.setHostName("127.0.0.1");
    torProxy.setPort(9050);
    QNetworkProxy::setApplicationProxy(torProxy);
    
    setTorEnabled(true); // also emits networkProxyChanged() signal
}

void neroshop::ProxyManager::useI2PProxy() {
    QNetworkProxy i2pProxy;
    i2pProxy.setType(QNetworkProxy::Socks5Proxy);
    i2pProxy.setHostName("127.0.0.1");
    i2pProxy.setPort(4447);
    QNetworkProxy::setApplicationProxy(i2pProxy);
    
    setTorEnabled(false);
}

QNetworkAccessManager * neroshop::ProxyManager::getNetworkClearnet() const {
    return clearnetManager;
}

QNetworkAccessManager * neroshop::ProxyManager::getNetworkTor() const {
    return torManager;
}

QNetworkAccessManager * neroshop::ProxyManager::getNetworkI2P() const {
    return i2pManager;
}

QNetworkAccessManager * neroshop::ProxyManager::getNetwork() const {
    QNetworkProxyQuery query(QUrl("http://example.com"));
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::proxyForQuery(query);
    
    if (proxies.isEmpty()) {
        return clearnetManager;
    }
    
    foreach(const QNetworkProxy &proxy, proxies) {
        auto proxyType = proxy.type(); // https://doc.qt.io/qt-6/qnetworkproxy.html#ProxyType-enum
        auto proxyPort = proxy.port();
        //auto proxyHostName = proxy.hostName();
        qDebug() << "Proxy type:" << proxy.type();
        switch(proxyType) {
            case QNetworkProxy::DefaultProxy: return clearnetManager;
            case QNetworkProxy::Socks5Proxy: 
                if(proxyPort == 9050) {
                    return torManager;
                }
                if(proxyPort == 4447) {
                    return i2pManager;
                }
                return torManager; // default for Socks5
            case QNetworkProxy::NoProxy: return clearnetManager;
            case QNetworkProxy::HttpProxy: return nullptr;
            case QNetworkProxy::HttpCachingProxy: return nullptr;
            case QNetworkProxy::FtpCachingProxy: return nullptr;
            default: return clearnetManager;
        }
    }
    
    return nullptr;
}

namespace {
const std::string tor_archive_url { "https://archive.torproject.org/tor-package-archive/torbrowser" };
const std::string tor_browser_version { "14.5.3" }; // tor 0.4.8.16
const std::string tor_expert_bundle { "tor-expert-bundle-" + 
    neroshop::device::get_os() + "-" + 
    neroshop::device::get_architecture() + "-" + tor_browser_version + ".tar.gz" };
    
const std::string tor { tor_archive_url + "/" + tor_browser_version + "/" + tor_expert_bundle };

}
void neroshop::ProxyManager::downloadTor() {
    if(hasTor()) {
        std::string torDirPath = neroshop::get_default_config_path() + "/tor";
        std::cout << "Tor found in " << torDirPath << std::endl;
        return;
    }
    
    std::string save_as { neroshop::get_default_config_path() + "/" + tor_expert_bundle };//"tor.tar.gz" };
    std::cout << "Downloading " << tor << " to " << neroshop::get_default_config_path() << "...\n\n";
    
    QNetworkAccessManager * manager = new QNetworkAccessManager(nullptr);
    QNetworkReply * reply = manager->get(QNetworkRequest(QUrl(QString::fromStdString(tor))));

    QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        qInfo() << "Download progress:" << bytesReceived << "/" << bytesTotal;
    });
    
    // Declare file outside the lambda function's scope
    QFile* file = new QFile(QString::fromStdString(save_as));
    
    // Connect to the finished signal to write the file
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // If download succeeds, write the file
            if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                qDebug() << "Failed to open file for writing:" << file->errorString();
                delete file;
                delete reply;
                delete manager;
                return;
            }
            file->write(reply->readAll());
            qInfo() << "Download succeeded";
            file->flush();
            file->close();
            delete file;
            
            // Extract the file
            if(neroshop::filesystem::is_file(save_as)) { 
                extractTar(QString::fromStdString(save_as));
            } else {
                qWarning() << "Error: Downloaded file not found\n";
            }
        } else {
            qWarning() << "Download failed:" << reply->errorString();
            delete file;
            delete reply;
            delete manager;
            return;
        }
        
        // Clean up resources
        reply->deleteLater();
        manager->deleteLater();// or delete manager;

    });
}

void neroshop::ProxyManager::extractTar(const QString& fileName) {
    auto fileNameStdString = fileName.toStdString();
    std::cout << "Extracting " << "tor/ from " << fileNameStdString << "...\n";
    
    std::string folder = fileNameStdString.substr(0, fileNameStdString.find_last_of("\\/")); // get path from filename
    assert(neroshop::filesystem::is_directory(folder));
    //--------------------------------------------------
    QString extractDir = QString::fromStdString(folder);
    QString wildcards = "tor/*";

    QProcess process;
    process.setProgram("tar");
        
    QStringList arguments;
    arguments << "-xzf" << fileName << "-C" << extractDir << "--wildcards" << wildcards;
    process.setArguments(arguments);

    process.start();
    process.waitForFinished();

    if(process.exitCode() != 0) {
        qDebug() << process.readAllStandardError();
    }
    //--------------------------------------------------
    // Check if tor/tor file was extracted or not
    std::string out_file { folder + "/" + "tor/tor" };
    
    if(!neroshop::filesystem::is_file(out_file)) {
        std::cout << "Error extracting tar\n"; 
        return;// or try a different method?
    }
    std::cout << "Extraction completed!\n";
    std::remove(fileNameStdString.c_str()); // Delete the tar.gz file after we're done extracting
}

void neroshop::ProxyManager::startTorDaemon() {
    std::string torDirPath = neroshop::get_default_config_path() + "/tor";
    #ifdef Q_OS_WIN
    QString program = QString::fromStdString(torDirPath + "/" + "tor.exe");
    #else
    QString program = QString::fromStdString(torDirPath + "/" + "./tor");
    #endif
    
    QStringList arguments;
    arguments << "-f" << QString::fromStdString(torDirPath + "/torrc");
    
    if(isTorRunning()) {
        std::cout << "\033[90mtor was already running in the background\033[0m\n";
        useTorProxy();
        setExternalProcess(true);
        return;
    }
    
    // Start Tor
    if(!torProcess) {
        torProcess = new QProcess(this);
        QObject::connect(torProcess, &QProcess::started, [&]() { // only works when placed before calling torProcess->start()
            emit processStarted();
        });
    }
        
    torProcess->start(program, arguments);
    if (torProcess->waitForStarted()) {
        qint64 pid = torProcess->processId();
        std::cout << "\033[90;1mtor started (pid: " << pid << ")\033[0m\n";
        useTorProxy();
        setExternalProcess(false);
    } else {
        torOutput.append(QString("%1 is missing so Tor could not be started\n").arg(program));
        emit torOutputChanged(torOutput);
        return;
    }
    
    // Connect the readyReadStandardOutput signal to a slot
    QObject::connect(torProcess, &QProcess::readyReadStandardOutput, [&]() {
        // Read the output of the process
        QByteArray output = torProcess->readAllStandardOutput();
        // Convert the output to QString and print it
        ////qDebug() << "Process output:" << QString::fromUtf8(output);
        // Convert the output to QString and store it in a member variable
        torOutput.append(QString::fromUtf8(output));
        emit torOutputChanged(torOutput); // Emit signal when the output changes
    });

    // Connect the finished signal to a slot
    QObject::connect(torProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [&](int exitCode, QProcess::ExitStatus exitStatus) {
        // Handle process finished event
        qDebug() << "Process finished with exit code:" << exitCode;
        setTorEnabled(false); // switch to useDefaultProxy() or?
        emit processFinished(exitCode, exitStatus);
    });
}

void neroshop::ProxyManager::stopTorDaemon() {
    if(torProcess) {
        torProcess->kill(); // Terminate the process
        torProcess->deleteLater(); // Delete the QProcess instance
        torProcess = nullptr;
    }
}

void neroshop::ProxyManager::setExternalProcess(bool externalProcess) {
    if(m_externalProcess != externalProcess) {
        m_externalProcess = externalProcess;
        emit processChanged();
    }
}

void neroshop::ProxyManager::setTorEnabled(bool torEnabled) {
    if(m_torEnabled != torEnabled) {
        m_torEnabled = torEnabled;
        emit networkProxyChanged();
        std::cout << ((m_torEnabled == true) ? "Tor is now enabled.\n" : "Tor has been disabled\n");
    }
}

QString neroshop::ProxyManager::getTorOutput() const {
    return torOutput;
}

QNetworkReply * neroshop::ProxyManager::getUrl(const QString& url) {
    auto network = getNetwork();
    return network->get(QNetworkRequest(QUrl(url)));
}

bool neroshop::ProxyManager::hasTor() {
    std::string torDirPath = neroshop::get_default_config_path() + "/tor";
    #ifdef Q_OS_WIN
    std::string torExecutable = torDirPath + "/" + "tor.exe";
    #else
    std::string torExecutable = torDirPath + "/" + "tor";
    #endif
    
    return neroshop::filesystem::is_file(torExecutable);
}

bool neroshop::ProxyManager::isTorRunning() {
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 9050); // Connect to Tor's SOCKS proxy port

    if (!socket.waitForConnected()) {
        QString errorMessage = socket.errorString();
        if (errorMessage.contains("Address already in use")) {
            return true;
        } else {
            //qDebug() << "Failed to connect to Tor's SOCKS proxy port:" << errorMessage;
            return false;
        }
    } else {
        // Close the socket
        socket.close();
        return true;
    }
}

bool neroshop::ProxyManager::isExternalProcess() const {
    return m_externalProcess;
}

bool neroshop::ProxyManager::isTorEnabled() const {
    return m_torEnabled;
}

void neroshop::ProxyManager::onReplyFinished(QNetworkReply * reply) {
    if(reply->error() == QNetworkReply::NoError) {
        // Reply received successfully
        QByteArray data = reply->readAll();
        ////qDebug() << "Data received:" << data;
        QVariant fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);
        qDebug() << "page from cache?" << fromCache.toBool();
    } else {
        // Error occurred
        qDebug() << "Network error: " << reply->errorString();
    }
    // Clean up the reply
    reply->deleteLater();
}

