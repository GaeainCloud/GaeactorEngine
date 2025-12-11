#ifndef LISTVIEWITEM_H
#define LISTVIEWITEM_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include "head_define.h"

#include "LocationHelper.h"

typedef std::tuple<bool,QString,QString,EVENT_INFO,LAT_LNG,LAT_LNG> LISTVIEW_ITEM_TYPE;
class DataSrcListViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DataSrcListViewModel(QObject *parent = nullptr);
    ~DataSrcListViewModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;

    void add(const EVENT_INFO& mapitem,const QString& sendorname,const QString& entityname, bool bIdentifi = true);
    void updateItem(const EVENT_INFO& mapitem,const QString& sendorname,const QString& entityname);
    const LISTVIEW_ITEM_TYPE& getItem(int indexrow) const;
    const QList<LISTVIEW_ITEM_TYPE>& getData() const;
    void remove(int index);
    void refresh();
    void clearData();
private:

    QList<LISTVIEW_ITEM_TYPE> m_dataSourceList;
};

class DataSrcItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DataSrcItemDelegate(DataSrcListViewModel *studentListViewModel, QObject *parent = nullptr);
    ~DataSrcItemDelegate() override;
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    DataSrcListViewModel *m_mapListViewModel;
};


#endif // LISTVIEWITEM_H
