#pragma once

#ifndef SETTINGS_MANAGER_HPP_NEROSHOP
#define SETTINGS_MANAGER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QSettings>
#endif

namespace neroshop {

class SettingsManager : public QObject {
    Q_OBJECT
public:
    static SettingsManager& instance();

    // JSON
    Q_INVOKABLE void saveJson(const QString& settings);
    Q_INVOKABLE QString getJsonString(const QString& key);
    Q_INVOKABLE int getJsonInt(const QString& key);
    Q_INVOKABLE bool getJsonBool(const QString& key);
    Q_INVOKABLE double getJsonDouble(const QString& key);
    Q_INVOKABLE QVariantList getJsonArray(const QString& key);
    Q_INVOKABLE QVariantMap getJsonRootObject() const; // root object
    Q_INVOKABLE QVariantMap getJsonObject(const QString& key);
    Q_INVOKABLE QString getJsonLiteral();
    QJsonObject getJsonRootObjectCpp() const;
    
    // QSettings
    void initializeDefaults();
    QString fileName() const;
    
    Q_INVOKABLE void setString(const QString &key, const QString &value);
    Q_INVOKABLE void setInt(const QString &key, int value);
    Q_INVOKABLE void setBool(const QString &key, bool value);

    Q_INVOKABLE QString getString(const QString &key, const QString &defaultValue = QString()) const;
    Q_INVOKABLE int getInt(const QString &key, int defaultValue = 0) const;
    Q_INVOKABLE bool getBool(const QString &key, bool defaultValue = false) const;
    
private:
    explicit SettingsManager(QObject *parent = nullptr);
    QJsonObject m_jsonObject;
    QSettings m_settings;
    // Disable copy and assignment to enforce singleton
    SettingsManager(SettingsManager const&) = delete;
    void operator=(SettingsManager const&) = delete;
};

}
#endif
