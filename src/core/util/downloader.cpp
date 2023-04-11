#include "downloader.hpp"

#if defined(NEROSHOP_USE_QT)
#include <QUrl>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#else
#include <curl/curl.h>
/* For older cURL versions you will also need 
#include <curl/types.h>
#include <curl/easy.h>
*/
#endif

#include "extractor.hpp" // ../util.hpp included here
#include "../config.hpp" // for config path
#include "../util.hpp"

namespace {
const std::string tor_archive_url { "https://archive.torproject.org/tor-package-archive/torbrowser" };
const std::string tor_browser_version { "12.0.4" }; // tor 0.4.7.13
const std::string tor_expert_bundle { "tor-expert-bundle-" + tor_browser_version + "-" + 
    neroshop::device::get_os() + "-" + 
    neroshop::device::get_architecture() + ".tar.gz" };

}

#if !defined(NEROSHOP_USE_QT)
size_t neroshop::tools::downloader::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
#endif

void neroshop::tools::downloader::download_tor() {
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    // check if tor already exists, if so then exit function
    if(neroshop::filesystem::is_file(config_path + "/" + "tor/tor")) {
        std::cout << "Tor has already been installed at " << (config_path + "/" + "tor/tor") << "\n"; return;
    }

    std::string tor { tor_archive_url + "/" + tor_browser_version + "/" + tor_expert_bundle };
        
    std::string save_as { config_path + "/" + tor_expert_bundle };//"tor.tar.gz" };
    std::cout << "downloading " << tor << " to " << config_path << "\n\n";
    #if defined(NEROSHOP_USE_QT)
    QNetworkAccessManager * manager = new QNetworkAccessManager(nullptr);
    QNetworkReply * reply = manager->get(QNetworkRequest(QUrl(QString::fromStdString(tor))));

    QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        qInfo() << "Download progress:" << bytesReceived << "/" << bytesTotal;
    });

    QFile* file = new QFile(QString::fromStdString(save_as));
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open file for writing:" << file->errorString();
        delete file;
        delete manager;
        return;
    }
    // Connect to the finished signal to write the file
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            file->write(reply->readAll());
            qInfo() << "Download succeeded";
        } else {
            qWarning() << "Download failed:" << reply->errorString();
        }
        file->flush();
        file->close();
        delete file;
        reply->deleteLater();
        manager->deleteLater();// or delete manager;
        // Extract the file
        if(!neroshop::filesystem::is_file(save_as)) { std::cout << "Error downloading tor\n"; return; }
        neroshop::tools::extractor::extract_tar(save_as);
    });
    #else
    CURL *curl;
    FILE *fp;
    CURLcode res;
    const char * outfilename = save_as.c_str();
    assert(save_as.length() <= FILENAME_MAX); // FILENAME_MAX=4096
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, tor.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    // Extract the file
    if(!neroshop::filesystem::is_file(save_as)) { std::cout << "Error downloading tor\n"; return; }
    neroshop::tools::extractor::extract_tar(save_as);
    #endif
}

void neroshop::tools::downloader::download_i2pd() {
    // TODO: use libi2pd instead
}
