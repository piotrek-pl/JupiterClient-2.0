#include "friendsstatuses.h"
#include "loginpage.h"
//#include "queries.h"
#include "mainwindow.h"

FriendsStatuses::FriendsStatuses(QObject *parent)
    : QObject{parent}
{
    database = QSqlDatabase::addDatabase("QMYSQL", "friendStatusesThread");
}

void FriendsStatuses::run()
{
    checkStatuses();
}

/*void FriendsStatuses::checkStatuses()
{
    QMap<quint32, Friend *> friendsMap = MainWindow::friendsMap;
    QMap<quint32, bool> availabilityFriendsMap;
    QMap<quint32, bool> messagesStatusesMap;

    for (auto it = friendsMap.begin(); it != friendsMap.end(); ++it)
    {
        quint32 key = it.key();
        Friend *value = it.value();

        availabilityFriendsMap.insert(key, value->isAvailable());
        messagesStatusesMap.insert(key, value->isNewMessage());
    }

    QMap<quint32, bool> actualAvailabilityFriendsMap = getAvailabilityFriendsMap();
    QMap<quint32, bool> actualMessagesStatusesMap = getMessagesStatusesMap();

    while (true)
    {
        if (actualAvailabilityFriendsMap != availabilityFriendsMap)
        {
            QMapIterator<quint32, bool> it1(actualAvailabilityFriendsMap);
            QMapIterator<quint32, bool> it2(availabilityFriendsMap);

            while (it1.hasNext() && it2.hasNext())
            {
                it1.next();
                it2.next();
                if (it1.value() != it2.value())
                {
                    emit availabilityStatusChanged(it1.key(), it1.value());
                }
            }
            availabilityFriendsMap = actualAvailabilityFriendsMap;
        }
        actualAvailabilityFriendsMap = getAvailabilityFriendsMap();

        if (actualMessagesStatusesMap != messagesStatusesMap)
        {
            QMapIterator<quint32, bool> it1(actualMessagesStatusesMap);
            QMapIterator<quint32, bool> it2(messagesStatusesMap);

            while (it1.hasNext() && it2.hasNext())
            {
                it1.next();
                it2.next();

                if (it1.value() != it2.value())
                {
                    emit messageStatusChanged(it1.key(), it1.value());
                }
            }
            messagesStatusesMap = actualMessagesStatusesMap;
        }
        actualMessagesStatusesMap = getMessagesStatusesMap();
    }
}*/

void FriendsStatuses::checkStatuses()
{
    QList<Friend *> friends = MainWindow::friends;
    QMap<quint32, bool> availabilityFriendsMap;
    QMap<quint32, bool> messagesStatusesMap;

    for (Friend *friendPtr : friends)
    {
        availabilityFriendsMap.insert(friendPtr->getId(), friendPtr->isAvailable());
        messagesStatusesMap.insert(friendPtr->getId(), friendPtr->isNewMessage());
    }

    QMap<quint32, bool> actualAvailabilityFriendsMap = getAvailabilityFriendsMap();
    QMap<quint32, bool> actualMessagesStatusesMap = getMessagesStatusesMap();


    while (true)
    {
        if (actualAvailabilityFriendsMap != availabilityFriendsMap)
        {
            QMapIterator<quint32, bool> it1(actualAvailabilityFriendsMap);
            QMapIterator<quint32, bool> it2(availabilityFriendsMap);

            while (it1.hasNext() && it2.hasNext())
            {
                it1.next();
                it2.next();
                if (it1.value() != it2.value())
                {
                    emit availabilityStatusChanged(it1.key(), it1.value());
                }
            }
            availabilityFriendsMap = actualAvailabilityFriendsMap;
        }
        actualAvailabilityFriendsMap = getAvailabilityFriendsMap();

        if (actualMessagesStatusesMap != messagesStatusesMap)
        {
            QMapIterator<quint32, bool> it1(actualMessagesStatusesMap);
            QMapIterator<quint32, bool> it2(messagesStatusesMap);

            while (it1.hasNext() && it2.hasNext())
            {
                it1.next();
                it2.next();

                if (it1.value() != it2.value())
                {
                    emit messageStatusChanged(it1.key(), it1.value());
                }
            }
            messagesStatusesMap = actualMessagesStatusesMap;
        }
        actualMessagesStatusesMap = getMessagesStatusesMap();
    }
}

QMap<quint32, bool> FriendsStatuses::getAvailabilityFriendsMap()
{
    QMap<quint32, bool> availabilityFriendsMap;
    QString friendsAvailability = QString("SELECT %1_friends.id, "
                                          "users.available "
                                          "FROM %1_friends "
                                          "JOIN users ON %1_friends.id = users.id")
                                    .arg(LoginPage::getUser().getId());
    LoginPage::connectToDatabase(database);
    QSqlQuery query(database);

    if (query.exec(friendsAvailability))
    {
        while (query.next())
        {
            availabilityFriendsMap.insert(query.value("id").toUInt(),
                                          query.value("available").toBool());
        }
    }


    return availabilityFriendsMap;
}

QMap<quint32, bool> FriendsStatuses::getMessagesStatusesMap()
{
    QMap<quint32, bool> messagesStatusesMap;
    QString messagesStatuses = QString("SELECT %1_friends.id, "
                                              "%1_friends.is_new_message "
                                              "FROM %1_friends")
                                        .arg(LoginPage::getUser().getId());
    LoginPage::connectToDatabase(database);
    QSqlQuery query(database);

    if (query.exec(messagesStatuses))
    {
        while (query.next())
        {
            messagesStatusesMap.insert(query.value("id").toUInt(),
                                       query.value("is_new_message").toBool());
        }
    }
    return messagesStatusesMap;
}

/*void FriendsStatuses::checkStatuses()
{
    QList<bool> actualFriendsList = getFriendsListFromDatabase();
    friendsList = actualFriendsList;

    QList<bool> actualMessagesStatusesList = getMessagesStatusesListFromDatabase();
    messagesStatusesList = actualMessagesStatusesList;

    while (true)
    {
        if (actualFriendsList != friendsList)
        {
            emit statusChanged();
            friendsList = actualFriendsList;
        }
        actualFriendsList = getFriendsListFromDatabase();

        if (actualMessagesStatusesList != messagesStatusesList)
        {
            emit statusChanged();
            messagesStatusesList = actualMessagesStatusesList;
        }
        actualMessagesStatusesList = getMessagesStatusesListFromDatabase();
    }
}*/

/*QList<bool> FriendsStatuses::getFriendsListFromDatabase()
{
    Queries queries;
    LoginPage::connectToDatabase(database);
    QSqlQuery query(database);
    QList<bool> friendsList;

    if (query.exec(queries.allFriendsQuery))
    {
        while (query.next())
        {
            friendsList.append(query.value("available").toBool());
        }
    }

    return friendsList;
}*/

/*QList<bool> FriendsStatuses::getMessagesStatusesListFromDatabase()
{
    Queries queries;
    LoginPage::connectToDatabase(database);
    QSqlQuery query(database);
    QList<bool> messagesStatusesList;

    if (query.exec(queries.newMessagesQuery))
    {
        while (query.next())
        {
            messagesStatusesList.append(query.value("is_new_message").toBool());
        }
    }

    return messagesStatusesList;
}*/

