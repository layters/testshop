#pragma once

#ifndef SCRIPT_MANAGER_HPP_NEROSHOP
#define SCRIPT_MANAGER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariant>
#endif

namespace neroshop {

class ScriptManager : public QObject {
    Q_OBJECT 
    // properties (for use in QML)
    //Q_PROPERTY(<type> <variable_name> READ ...)
public:
    ScriptManager(QObject *parent = nullptr);

    // functions (for use in QML)
    //Q_INVOKABLE <type> getTable(const QString& key) const {}
    Q_INVOKABLE QString getString(const QString& key) const;// {}
    //Q_INVOKABLE <type> getUserdata(const QString& key) const {}
    Q_INVOKABLE double getNumber(const QString& key) const;// {}
    Q_INVOKABLE bool getBoolean(const QString& key) const;// {}
    //Q_INVOKABLE <type> get_(const QString& key) const {}
    Q_INVOKABLE QVariantList getTableStrings(const QString& key) const;
    
    Q_INVOKABLE void saveJson(const QString& settings);
    Q_INVOKABLE QString getJsonString(const QString& key);
    Q_INVOKABLE int getJsonInt(const QString& key);
    Q_INVOKABLE bool getJsonBool(const QString& key);
    Q_INVOKABLE double getJsonDouble(const QString& key);
    Q_INVOKABLE QVariantList getJsonArray(const QString& key);
    Q_INVOKABLE QVariantMap getJsonRootObject() const; // root object
    Q_INVOKABLE QVariantMap getJsonObject(const QString& key);
    Q_INVOKABLE QString getJsonLiteral();
    //Q_INVOKABLE ? getJson(const QString& key);
    QJsonObject getJsonRootObjectCpp() const;
private:
    QJsonObject json_object;
};

}
#endif
