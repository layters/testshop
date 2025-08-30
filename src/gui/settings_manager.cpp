#include "settings_manager.hpp"

#include "../neroshop_config.hpp"   // get_default_config_path()
#include "../core/tools/script.hpp"
#include "../core/settings.hpp"     // neroshop::lua_state

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace neroshop {

//-----------------------------------------------------------------------------

SettingsManager::SettingsManager(QObject *parent) : QObject(parent),
      m_settings(QString::fromStdString(get_default_config_path()) + "/neroshop.conf", QSettings::IniFormat) {
    neroshop::create_json(); // Create settings.json if does not yet exist
    QString json = QString::fromStdString(neroshop::load_json());
    QJsonParseError json_error;
    const auto json_doc = QJsonDocument::fromJson(json.toUtf8(), &json_error);
    if(json_error.error != QJsonParseError::NoError) {
        throw std::runtime_error("Error parsing settings.json");
    }
    assert(json_doc.isObject());
    m_jsonObject = json_doc.object();
}

//SettingsManager::~Script() {}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance(nullptr); // no parent - independent and managed by C++
    return instance;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QString SettingsManager::getJsonString(const QString& key) {
    QJsonValue json_value = m_jsonObject.value(key);
    assert(json_value.isString());
    return json_value.toString();
}

int SettingsManager::getJsonInt(const QString& key) {
    QJsonValue json_value = m_jsonObject.value(key);
    //assert(json_value.is?());
    return json_value.toInt();
}

bool SettingsManager::getJsonBool(const QString& key) {
    QJsonValue json_value = m_jsonObject.value(key);
    assert(json_value.isBool());
    return json_value.toBool();
}

double SettingsManager::getJsonDouble(const QString& key) {
    QJsonValue json_value = m_jsonObject.value(key);
    assert(json_value.isDouble());
    return json_value.toDouble();
}

QVariantList SettingsManager::getJsonArray(const QString& key) {
    QJsonValue json_value = m_jsonObject.value(key);
    assert(json_value.isArray());
    QJsonArray json_array = json_value.toArray();
    return json_array.toVariantList();
}

QVariantMap SettingsManager::getJsonObject(const QString& key) {
    QJsonValue json_value = m_jsonObject.value(key);
    assert(json_value.isObject());
    QJsonObject json_obj = json_value.toObject();
    return json_obj.toVariantMap();
}

QVariantMap SettingsManager::getJsonRootObject() const {
    return m_jsonObject.toVariantMap();
}

QString SettingsManager::getJsonLiteral() {
    QJsonDocument json_doc(m_jsonObject);
    return QString(json_doc.toJson(QJsonDocument::Compact));
}

void SettingsManager::saveJson(const QString& settings) {
    neroshop::modify_json(settings.toStdString());
}

QJsonObject SettingsManager::getJsonRootObjectCpp() const {
    return m_jsonObject;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SettingsManager::initializeDefaults() {
    if (!m_settings.contains("preferred_currency")) {
        m_settings.setValue("preferred_currency", "USD");
    }
    if (!m_settings.contains("dark_theme")) {
        m_settings.setValue("dark_theme", true);
    }
    if (!m_settings.contains("theme")) {
        m_settings.setValue("theme", "PurpleDust");
    }
    if (!m_settings.contains("language")) {
        m_settings.setValue("language", "English");
    }
    if (!m_settings.contains("hide_homepage_button")) {
        m_settings.setValue("hide_homepage_button", false);
    }
    if (!m_settings.contains("hide_price_display")) {
        m_settings.setValue("hide_price_display", false);
    }
    if (!m_settings.contains("hide_wallet_sync_bar_on_full")) {
        m_settings.setValue("hide_wallet_sync_bar_on_full", true);
    }
    if (!m_settings.contains("wallet_directory")) {
        m_settings.setValue("wallet_directory", "");
    }
    if (!m_settings.contains("last_opened_wallet")) {
        m_settings.setValue("last_opened_wallet", "");
    }
    if (!m_settings.contains("remember_wallet")) {
        m_settings.setValue("remember_wallet", false);
    }

    // Catalog group
    if (!m_settings.contains("catalog/price_display")) {
        m_settings.setValue("catalog/price_display", "All prices");
    }
    if (!m_settings.contains("catalog/hide_product_details")) {
        m_settings.setValue("catalog/hide_product_details", false);
    }
    if (!m_settings.contains("catalog/catalog_view")) {
        m_settings.setValue("catalog/catalog_view", "Grid view");
    }
    if (!m_settings.contains("catalog/grid_details_align_center")) {
        m_settings.setValue("catalog/grid_details_align_center", false);
    }
    if (!m_settings.contains("catalog/hide_illegal_products")) {
        m_settings.setValue("catalog/hide_illegal_products", true);
    }
    if (!m_settings.contains("catalog/hide_nsfw_products")) {
        m_settings.setValue("catalog/hide_nsfw_products", false);
    }

    // Monero daemon group
    if (!m_settings.contains("monero/daemon/confirm_external_bind")) {
        m_settings.setValue("monero/daemon/confirm_external_bind", false);
    }
    if (!m_settings.contains("monero/daemon/restricted_rpc")) {
        m_settings.setValue("monero/daemon/restricted_rpc", true);
    }
    if (!m_settings.contains("monero/daemon/data_dir")) {
        m_settings.setValue("monero/daemon/data_dir", "");
    }
    if (!m_settings.contains("monero/daemon/auto_sync")) {
        m_settings.setValue("monero/daemon/auto_sync", true);
    }
    if (!m_settings.contains("monero/daemon/node_type")) {
        m_settings.setValue("monero/daemon/node_type", 0);
    }
    if (!m_settings.contains("monero/daemon/executable")) {
        m_settings.setValue("monero/daemon/executable", "");
    }
    if (!m_settings.contains("monero/daemon/last_selected_node")) {
        m_settings.setValue("monero/daemon/last_selected_node", "");
    }

    // Monero wallet group
    if (!m_settings.contains("monero/wallet/balance_display")) {
        m_settings.setValue("monero/wallet/balance_display", "All balances");
    }
    if (!m_settings.contains("monero/wallet/balance_amount_precision")) {
        m_settings.setValue("monero/wallet/balance_amount_precision", 12);
    }
    if (!m_settings.contains("monero/wallet/show_currency_sign")) {
        m_settings.setValue("monero/wallet/show_currency_sign", false);
    }
    if (!m_settings.contains("monero/wallet/block_explorer")) {
        m_settings.setValue("monero/wallet/block_explorer", "xmrchain.net");
    }
    /*if (!m_settings.contains("monero/wallet/require_password_on_withdrawal")) {
        m_settings.setValue("monero/wallet/require_password_on_withdrawal", true);
    }*/
    if (!m_settings.contains("monero/wallet/seed_language")) {
        m_settings.setValue("monero/wallet/seed_language", "English");
    }

    // Data expiration group
    if (!m_settings.contains("data_expiration/listing")) {
        m_settings.setValue("data_expiration/listing", "Never");
    }
    if (!m_settings.contains("data_expiration/product_rating")) {
        m_settings.setValue("data_expiration/product_rating", "Never");
    }
    if (!m_settings.contains("data_expiration/seller_rating")) {
        m_settings.setValue("data_expiration/seller_rating", "Never");
    }
    if (!m_settings.contains("data_expiration/order")) {
        m_settings.setValue("data_expiration/order", "2 years");
    }
    if (!m_settings.contains("data_expiration/message")) {
        m_settings.setValue("data_expiration/message", "30 days");
    }

    // No need to call sync() explicitly here unless you want to guarantee storage at this point
    m_settings.sync();
}

//-----------------------------------------------------------------------------

QString SettingsManager::fileName() const {
    return m_settings.fileName();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SettingsManager::setString(const QString &key, const QString &value) {
    m_settings.setValue(key, value);
}

//-----------------------------------------------------------------------------

void SettingsManager::setInt(const QString &key, int value) {
    m_settings.setValue(key, value);
}

//-----------------------------------------------------------------------------

void SettingsManager::setBool(const QString &key, bool value) {
    m_settings.setValue(key, value);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QString SettingsManager::getString(const QString &key, const QString &defaultValue) const {
    return m_settings.value(key, defaultValue).toString();
}

//-----------------------------------------------------------------------------

int SettingsManager::getInt(const QString &key, int defaultValue) const {
    return m_settings.value(key, defaultValue).toInt();
}

//-----------------------------------------------------------------------------

bool SettingsManager::getBool(const QString &key, bool defaultValue) const {
    return m_settings.value(key, defaultValue).toBool();
}

//-----------------------------------------------------------------------------

}
