#pragma once

#ifndef WALLET_PROXY_HPP_NEROSHOP
#define WALLET_PROXY_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QObject>
#include <QString>
#endif
#include <memory> // std::unique_ptr

#include "../core/wallet.hpp"

namespace neroshop {

namespace gui { // or just remove the "gui" namespace and rename gui::Wallet to WalletProxy?

#if defined(NEROSHOP_USE_QT)
class Wallet : public QObject {
    Q_OBJECT 
    // properties (for use in QML)
    Q_PROPERTY(neroshop::Wallet* wallet READ get_wallet WRITE set_wallet);// NOTIFY wallet_changed);
    //Q_PROPERTY(<type> <variable_name> READ <get_function_name>)
public:    
    // functions (for use in QML)
    ////explicit Wallet(QObject* parent = 0);
    Q_INVOKABLE int create_random_wallet(const QString& password, const QString& confirm_pwd, const QString& path) const;
    Q_INVOKABLE QString get_mnemonic() const;
    Q_INVOKABLE neroshop::Wallet * get_wallet() const;
    Q_INVOKABLE void set_wallet(const neroshop::Wallet* wallet/*const neroshop::Wallet& wallet*/) {}//const { /*this->wallet = const_cast<neroshop::Wallet*>(wallet);*//*this->wallet = &const_cast<neroshop::Wallet&>(wallet);*//*emit wallet_changed(status);*/ }
    //Q_INVOKABLE <type> <function_name>() const {}
//signals:
//    void wallet_changed(int status);
//public slots:    
#else
class Wallet { 
#endif
private:
    std::unique_ptr<neroshop::Wallet> wallet;
public:
    Wallet();
    ~Wallet();
};

}

}

#endif
