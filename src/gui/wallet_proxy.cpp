#include "wallet_proxy.hpp"

neroshop::gui::Wallet::Wallet() {
    wallet = std::make_unique<neroshop::Wallet>();
}

neroshop::gui::Wallet::~Wallet() {
    if(wallet.get()) {
        wallet.reset();
    }
    #ifdef NEROSHOP_DEBUG
    std::cout << "wallet proxy deleted\n";
    #endif
}

#if defined(NEROSHOP_USE_QT)
////neroshop::gui::Wallet::Wallet(QObject* parent) : QObject(parent) {}

neroshop::Wallet * neroshop::gui::Wallet::getWallet() const {
    return wallet.get();
}

int neroshop::gui::Wallet::createRandomWallet(const QString& password, const QString& confirm_pwd, const QString& path) const {
    auto error = wallet->create_random(password.toStdString(), confirm_pwd.toStdString(), path.toStdString());
    return static_cast<int>(error);
}

void neroshop::gui::Wallet::copyMnemonicToClipboard() {
    if(!wallet->get_monero_wallet()) return;
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(getMnemonic());
    std::cout << "Copied to clipboard\n";
}

QString neroshop::gui::Wallet::getMnemonic() const {
    if(!wallet->get_monero_wallet()) return "";
    return QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic());
}

QStringList neroshop::gui::Wallet::getMnemonicModel() const {
    QStringList seed_phrase = QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic()).split(' ');
    return seed_phrase;
}

void neroshop::gui::Wallet::daemonExecute(const QString& ip, const QString& port, bool confirm_external_bind, bool restricted_rpc, bool remote, QString data_dir, QString network_type, unsigned int restore_height) {//const {
    wallet->daemon_open(ip.toStdString(), port.toStdString(), confirm_external_bind, restricted_rpc, remote, data_dir.toStdString(), network_type.toStdString(), restore_height);
}

void neroshop::gui::Wallet::daemonConnect(const QString& ip, const QString& port) {
    std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n";
    bool synced = wallet->daemon_connect(ip.toStdString(), port.toStdString());  
    std::cout << synced << "\n";  
    ////#include <thread> // since C++11
    ////#include <future> // since C++11
    /*std::future<bool> synced = std::async(std::launch::async, &neroshop::Wallet::daemon_connect, wallet.get(), ip.toStdString(), port.toStdString()); // NOTE: & is mandatory for member functions, but optional for free functions. // use either std::launch::async or std::launch::deferred
    std::cout << synced.get() << "\n";*/
    
    /*std::shared_ptr<neroshop::Wallet> wallet_ptr = std::shared_ptr<neroshop::Wallet>(std::move(wallet)); // moves unique_ptr to shared_ptr
    std::packaged_task<void(void)> sync_job([wallet_ptr, ip, port]() {
        wallet_ptr->daemon_connect(ip.toStdString(), port.toStdString());
    });
    // save the value
    std::future<void> job_value = sync_job.get_future(); // must have same return value as packaged_task
    // move the task (function) to a separate thread
    std::thread worker(std::move(sync_job));
    worker.join();
    // take back wallet from shared_ptr to unique_ptr
    ////this->wallet = std::unique_ptr<neroshop::Wallet>(wallet_ptr);*/
    
}

double neroshop::gui::Wallet::getSyncPercentage() const {
    return wallet->get_sync_percentage();
}

unsigned int neroshop::gui::Wallet::getSyncHeight() const {
    return wallet->get_sync_height();
}

unsigned int neroshop::gui::Wallet::getSyncStartHeight() const {
    return wallet->get_sync_start_height();
}

unsigned int neroshop::gui::Wallet::getSyncEndHeight() const {
    return wallet->get_sync_end_height();
}

QString neroshop::gui::Wallet::getSyncMessage() const {
    return QString::fromStdString(wallet->get_sync_message());
}


bool neroshop::gui::Wallet::isGenerated() const {
    return (wallet->get_monero_wallet() != nullptr);
}
#endif
