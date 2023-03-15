#pragma once

#ifndef TABLE_MODEL_HPP_NEROSHOP
#define TABLE_MODEL_HPP_NEROSHOP

#include <qqml.h>
#include <QAbstractTableModel>

// TODO: Switch to https://doc.qt.io/qt-5/qml-qt-labs-qmlmodels-tablemodel.html whenever upgrading to Qt 5.14
namespace neroshop {

class TableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_ADDED_IN_MINOR_VERSION(1)

public:
    int rowCount(const QModelIndex & = QModelIndex()) const override;

    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;
};

}
#endif
