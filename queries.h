#ifndef QUERIES_H
#define QUERIES_H

#include <QObject>

class Queries : public QObject
{
    Q_OBJECT
public:
    explicit Queries(QObject *parent = nullptr);
    QString availableFriendsQuery;
    QString unavailableFriendsQuery;
    QString allFriendsQuery;
    QString newMessagesQuery;
private:
    QString username;
};

#endif // QUERIES_H
