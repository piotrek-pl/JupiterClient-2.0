#ifndef EXTENDEDQLISTWIDGETITEM_H
#define EXTENDEDQLISTWIDGETITEM_H

#include <QListWidgetItem>

class ExtendedQListWidgetItem : public QListWidgetItem
{
public:
    ExtendedQListWidgetItem(const QIcon &icon, const QString &text, const quint32 id);
    quint32 getId() { return id; }

private:
    quint32 id;
};

Q_DECLARE_METATYPE(ExtendedQListWidgetItem*)

#endif // EXTENDEDQLISTWIDGETITEM_H
