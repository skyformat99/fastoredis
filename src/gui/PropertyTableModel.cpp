#include "gui/PropertyTableModel.h"

#include "gui/GuiFactory.h"
#include "common/qt/utils_qt.h"
#include "common/qt/converter_patterns.h"

namespace fastoredis
{
    PropertyTableItem::PropertyTableItem(const QString &key, const QString &value)
        : key_(key), value_(value)
    {

    }

    PropertyTableModel::PropertyTableModel(QObject *parent)
        : TableModel(parent)
    {

    }

    QVariant PropertyTableModel::data(const QModelIndex &index, int role) const
    {
        QVariant result;

        if (!index.isValid())
            return result;

        PropertyTableItem *node = common::utils_qt::item<PropertyTableItem*>(index);

        if (!node)
            return result;

        int col = index.column();

        if (role == Qt::DisplayRole) {
            if (col == PropertyTableItem::eKey) {
                result = node->key_;
            }
            else if (col == PropertyTableItem::eValue) {
                result = node->value_;
            }
        }
        return result;
    }

    bool PropertyTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::EditRole) {
            int row = index.row();
            int column = index.column();
            PropertyTableItem *node = common::utils_qt::item<PropertyTableItem*>(index);

            if (!node)
                return false;

            if (column == PropertyTableItem::eKey) {

            }
            else if (column == PropertyTableItem::eValue) {
                const QString &newValue = value.toString();
                if(newValue != node->value_){
                    PropertyType pr;
                    pr.first = common::utils_qt::toStdString(node->key_);
                    pr.second = common::utils_qt::toStdString(newValue);
                    emit changedProperty(pr);
                }
            }
        }

        return false;
    }

    void PropertyTableModel::changeProperty(const PropertyType &pr)
    {
        const QString &key = common::utils_qt::toQString(pr.first);
        for(int i = 0; i < data_.size(); ++i)
        {
            PropertyTableItem *it = dynamic_cast<PropertyTableItem*>(data_[i]);
            if(it->key_ == key){
                it->value_ = common::utils_qt::toQString(pr.second);
                emit dataChanged(index(i,0),index(i,1));
                break;
            }
        }
    }

    QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == PropertyTableItem::eKey) {
                return "Key";
            }
            else if (section == PropertyTableItem::eValue) {
                return "Value";
            }
        }

        return TableModel::headerData(section,orientation,role);
    }

    int PropertyTableModel::columnCount(const QModelIndex &parent) const
    {
        return PropertyTableItem::eCountColumns;
    }

    Qt::ItemFlags PropertyTableModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags result = 0;
        if (index.isValid()) {
            result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            int col = index.column();
            if(col == PropertyTableItem::eValue){
                result |= Qt::ItemIsEditable;
            }
        }
        return result;
    }

    void PropertyTableModel::addItem(PropertyTableItem *item)
    {
        beginInsertRows(QModelIndex(), data_.size(), data_.size()+1 );
        data_.push_back(item);
        endInsertRows();
    }

    PropertyTableModel::~PropertyTableModel()
    {

    }
}
