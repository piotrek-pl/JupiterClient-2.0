#include "extendedqlistwidgetitem.h"

ExtendedQListWidgetItem::ExtendedQListWidgetItem(const QIcon &icon, const QString &text, const quint32 id)
    : QListWidgetItem(icon, text),
      id(id)
{

}
