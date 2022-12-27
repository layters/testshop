#pragma once

#ifndef WALLET_CONTROLLER_HPP_NEROSHOP
#define WALLET_CONTROLLER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#endif
#include <memory> // std::unique_ptr

#include "../core/wallet.hpp"

namespace neroshop {

#if defined(NEROSHOP_USE_QT)
class WalletController : public QObject, public neroshop::Wallet {
    Q_OBJECT 
    // properties (for use in QML)
    Q_PROPERTY(neroshop::Wallet* wallet READ getWallet);// NOTIFY wallet_changed);
    //Q_PROPERTY(<type> <variable_name> READ <get_function_name>)
public:    
    // I don't know how to use or compare enums in QML. It never works, but oh well :|
    enum KeyfileStatus {
        KeyfileStatus_Ok = 0,
        KeyfileStatus_Wrong_Password,
        KeyfileStatus_No_Matching_Passwords,
        KeyfileStatus_Exists,
    };
    Q_ENUM(KeyfileStatus)
    // functions (for use in QML)
    ////explicit Wallet(QObject* parent = 0);
    Q_INVOKABLE int createRandomWallet(const QString& password, const QString& confirm_pwd, const QString& path) const;
    Q_INVOKABLE void restoreFromMnemonic(const QString& mnemonic);
    //Q_INVOKABLE void restoreFromKeys(const QString& primary_address, const QString& private_view_key, const QString& private_spend_key());
    Q_INVOKABLE bool open(const QString& path, const QString& password);
    Q_INVOKABLE void close(bool save = false);
    Q_INVOKABLE QVariantMap/*QMap<QString, QVariant>*/ createUniqueSubaddressObject(unsigned int account_idx, const QString & label = "");
    Q_INVOKABLE QString signMessage(const QString& message) const;
    Q_INVOKABLE bool verifyMessage(const QString& message, const QString& signature) const;
    
    Q_INVOKABLE double getSyncPercentage() const;
    Q_INVOKABLE unsigned int getSyncHeight() const;
    Q_INVOKABLE unsigned int getSyncStartHeight() const;
    Q_INVOKABLE unsigned int getSyncEndHeight() const;
    Q_INVOKABLE QString getSyncMessage() const;
    Q_INVOKABLE int getNetworkType() const;
    Q_INVOKABLE QString getNetworkTypeString() const;
    Q_INVOKABLE QString getMnemonic() const;
    Q_INVOKABLE QStringList getMnemonicList() const;
    Q_INVOKABLE QString getPrimaryAddress() const;
    // todo: change getAddresses* functions to return a QVariantList (array) containing QVariantMaps (objects) that represent a monero subaddress
    Q_INVOKABLE QStringList getAddressesAll() const;
    Q_INVOKABLE QStringList getAddressesUsed() const;
    Q_INVOKABLE QStringList getAddressesUnused() const;
    Q_INVOKABLE double getBalanceLocked(unsigned int account_index) const;
    Q_INVOKABLE double getBalanceLocked(unsigned int account_index, unsigned int subaddress_index) const;
    Q_INVOKABLE double getBalanceUnlocked(unsigned int account_index) const;
    Q_INVOKABLE double getBalanceUnlocked(unsigned int account_index, unsigned int subaddress_index) const;
    Q_INVOKABLE neroshop::Wallet * getWallet() const;
    
    Q_INVOKABLE void setNetworkTypeByString(const QString& network_type);
    //Q_INVOKABLE <type> <function_name>() const {}
    Q_INVOKABLE bool isOpened() const;
    Q_INVOKABLE bool isConnectedToDaemon() const;
    Q_INVOKABLE bool isSynced() const;
    Q_INVOKABLE bool isDaemonSynced() const;
    Q_INVOKABLE bool fileExists(const QString& filename) const;
    
    Q_INVOKABLE void nodeConnect(const QString& ip, const QString& port, const QString& username = "", const QString& password = "");
    Q_INVOKABLE void daemonConnect(const QString& username = "", const QString& password = "");
    Q_INVOKABLE void daemonExecute(const QString& daemon_dir, bool confirm_external_bind, bool restricted_rpc, QString data_dir, unsigned int restore_height);

    //Q_INVOKABLE QString signMessage() const;
    //Q_INVOKABLE bool verifyMessage() const;
public slots: 
signals:
#else
class WalletController { 
#endif
public://private:
    std::unique_ptr<neroshop::Wallet> wallet;
public:
    WalletController();
    ~WalletController();
};

}

#endif