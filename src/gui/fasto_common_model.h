#pragma once

#include "gui/base/tree_model.h"

#include "core/types.h"

namespace fastoredis
{
    class FastoCommonModel
            : public TreeModel
    {
        Q_OBJECT
    public:
        FastoCommonModel(QObject *parent = 0);
        ~FastoCommonModel();

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        virtual int columnCount(const QModelIndex& parent) const;

        void changeValue(const DbValue& value, const std::string& command);

    Q_SIGNALS:
        void changedValue(const DbValue& value, const std::string& command);
    };
}


