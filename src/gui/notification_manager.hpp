#pragma once

#ifndef NOTIFICATION_MANAGER_HPP_NEROSHOP
#define NOTIFICATION_MANAGER_HPP_NEROSHOP

#include <QObject>

namespace neroshop {

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    explicit NotificationManager(QObject *toastObject, QObject *parent = nullptr);

    Q_INVOKABLE void showToast(const QString &message);
    
    static NotificationManager* instance(QObject* toastObject = nullptr);

private:
    QObject *m_toastObject;
};

}

#endif
