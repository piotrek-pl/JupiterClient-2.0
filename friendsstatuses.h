#ifndef FRIENDSSTATUSES_H
#define FRIENDSSTATUSES_H

#include "databaseconnectionmanager.h"

#include <QObject>
#include <QSqlDatabase>

class FriendsStatuses : public QObject
{
    Q_OBJECT
public:
    explicit FriendsStatuses(QObject *parent = nullptr);
    DatabaseConnectionManager databaseConnectionManager;

signals:
    //void statusChanged();
    void availabilityStatusChanged(quint32, bool);
    void messageStatusChanged(quint32, bool);
    void removedFriend(quint32);
    void addedFriend(quint32);

public slots:
    void run();

private:
    QSqlDatabase database;
    QList<bool> friendsList;
    QList<bool> messagesStatusesList;
    void checkStatuses();
    QList<bool> getFriendsListFromDatabase();
    QList<bool> getMessagesStatusesListFromDatabase();
    QMap<quint32, bool> getAvailabilityFriendsMap();
    QMap<quint32, bool> getMessagesStatusesMap();
    QMap<quint32, bool> getFriendOfFriendMap();
    QList<quint32> getFriendsIdNumbersList();


};

#endif // FRIENDSSTATUSES_H
