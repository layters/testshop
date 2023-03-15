#include "table_model.hpp"

int neroshop::TableModel::rowCount(const QModelIndex &) const 
{
    return 200;
}

int neroshop::TableModel::columnCount(const QModelIndex &) const 
{
    return 200;
}

QVariant neroshop::TableModel::data(const QModelIndex &index, int role) const 
{
    switch (role) {
        case Qt::DisplayRole:
            return QString("%1, %2").arg(index.column()).arg(index.row());
        default:
            break;
    }

    return QVariant();
}

QHash<int, QByteArray> neroshop::TableModel::roleNames() const 
{
    return { {Qt::DisplayRole, "display"} };
}
