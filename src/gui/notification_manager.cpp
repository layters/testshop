#include "notification_manager.hpp"

#include <QMetaObject>
#include <QVariant>

namespace neroshop {

NotificationManager::NotificationManager(QObject *toastObject, QObject *parent)
    : QObject(parent), m_toastObject(toastObject)
{
}

void NotificationManager::showToast(const QString &message)
{
    if (m_toastObject) {
        QMetaObject::invokeMethod(m_toastObject, "showNotification",
                                  Q_ARG(QVariant, QVariant(message)));
    }
}

NotificationManager* NotificationManager::instance(QObject* toastObject) {
    static NotificationManager* _instance = nullptr;
    if (!_instance && toastObject)
        _instance = new NotificationManager(toastObject);
    return _instance;
}

} // namespace neroshop

