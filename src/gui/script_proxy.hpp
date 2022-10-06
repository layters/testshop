#pragma once

#ifndef SCRIPT_PROXY_HPP_NEROSHOP
#define SCRIPT_PROXY_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QObject>
#include <QString>
#include <QVariant>
#endif

#include "../core/script.hpp"
#include "../core/config.hpp" // neroshop::lua_state

namespace neroshop {

namespace gui { 

#if defined(NEROSHOP_USE_QT)
class Script : public QObject {
    Q_OBJECT 
    // properties (for use in QML)
    //Q_PROPERTY(<type> <variable_name> READ ...)
public:    
    // functions (for use in QML)
    //Q_INVOKABLE <type> getTable(const QString& key) const {}
    Q_INVOKABLE QString getString(const QString& key) const;// {}
    //Q_INVOKABLE <type> getUserdata(const QString& key) const {}
    Q_INVOKABLE double getNumber(const QString& key) const;// {}
    Q_INVOKABLE bool getBoolean(const QString& key) const;// {}
    //Q_INVOKABLE <type> get_(const QString& key) const {}
    // Containers
    Q_INVOKABLE QVariantList getTableStrings(const QString& key) const;
#else
class Script {    
#endif
public:
    //Script();
    //~Script();
};

}

}

#endif
