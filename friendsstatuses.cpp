#include "friendsstatuses.h"
#include "loginpage.h"
//#include "queries.h"
#include "mainwindow.h"

FriendsStatuses::FriendsStatuses(QObject *parent)
    : QObject{parent}
{
    database = QSqlDatabase::addDatabase("QMYSQL", "friendStatusesThread");
    LoginPage::connectToDatabase(database);
}

void FriendsStatuses::run()
{
    checkStatuses();
}

void FriendsStatuses::checkStatuses()
{
    QMap<quint32, Friend *> friendsMap = MainWindow::friendsMap;
    QMap<quint32, bool> availabilityFriendsMap;
    QMap<quint32, bool> messagesStatusesMap;
    QList<quint32> friendsIdNumbersList;


    for (const auto& id : friendsMap.keys())
    {
        availabilityFriendsMap.insert(id, friendsMap.value(id)->isAvailable());
        messagesStatusesMap.insert(id, friendsMap.value(id)->isNewMessage());
        friendsIdNumbersList.append(id);
    }

    QMap<quint32, bool> currentAvailabilityFriendsMap = getAvailabilityFriendsMap();
    QMap<quint32, bool> currentMessagesStatusesMap = getMessagesStatusesMap();
    QList<quint32> currentFriendsIdNumbersList = getFriendsIdNumbersList();

    while (true)
    {
        if (currentAvailabilityFriendsMap != availabilityFriendsMap)
        {
            QMapIterator<quint32, bool> it1(currentAvailabilityFriendsMap);
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
            availabilityFriendsMap = currentAvailabilityFriendsMap;
        }
        currentAvailabilityFriendsMap = getAvailabilityFriendsMap();

        if (currentMessagesStatusesMap != messagesStatusesMap)
        {
            QMapIterator<quint32, bool> it1(currentMessagesStatusesMap);
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
            messagesStatusesMap = currentMessagesStatusesMap;
        }
        currentMessagesStatusesMap = getMessagesStatusesMap();

        if (currentFriendsIdNumbersList != friendsIdNumbersList)
        {
            for (quint32 friendId : friendsIdNumbersList)
            {
                if (!currentFriendsIdNumbersList.contains(friendId))
                {
                    qDebug() << friendId;
                    emit removedFriend(friendId);
                }
            }
        }
        friendsIdNumbersList = currentFriendsIdNumbersList;
        currentFriendsIdNumbersList = getFriendsIdNumbersList();
    }

}


/*void FriendsStatuses::checkStatuses()
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
}*/

QMap<quint32, bool> FriendsStatuses::getAvailabilityFriendsMap()
{
    QMap<quint32, bool> availabilityFriendsMap;
    QString friendsAvailability = QString("SELECT %1_friends.id, "
                                          "users.available "
                                          "FROM %1_friends "
                                          "JOIN users ON %1_friends.id = users.id")
                                    .arg(LoginPage::getUser().getId());
    //LoginPage::connectToDatabase(database);
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
    //LoginPage::connectToDatabase(database);
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

QList<quint32> FriendsStatuses::getFriendsIdNumbersList()
{
    QList<quint32> friendsIdNumbersList;
    QString friendsIdNumbers = QString("SELECT %1_friends.id "
                                       "FROM %1_friends")
                                .arg(LoginPage::getUser().getId());

    QSqlQuery query(database);

    if (query.exec(friendsIdNumbers))
    {
        while (query.next())
        {
            friendsIdNumbersList.append(query.value("id").toUInt());
        }
    }

    return friendsIdNumbersList;

}
