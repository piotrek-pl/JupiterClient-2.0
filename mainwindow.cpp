#include <QThread>
#include <QHostAddress>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "friendsstatuses.h"
#include "queries.h"
#include "loginpage.h"
#include "chatwindow.h"
#include "extendedqlistwidgetitem.h"

QList<quint32> MainWindow::activeWindowsList;
QTcpSocket *MainWindow::socket;
QList<Friend *> MainWindow::friends;
//QMap<quint32, Friend *> MainWindow::friendsMap;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectToServer();
    friends = getFriendsList();
    //friendsMap = getFriendsMap();
    fillOutFriendsListWidget();
    //createFriendsList();
    makeThread();


}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::connectToServer()
{
    socket = new QTcpSocket();

    connect(socket, &QTcpSocket::connected, this, &MainWindow::socketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::socketDisconnected);
    socket->connectToHost("77.237.31.25", 1234);
}

void MainWindow::socketConnected()
{
    qDebug() << "Connected to server.";
    qDebug() << "localHostAddress =" << socket->localAddress().toString()
             << "localPort =" << socket->localPort();

    sendFirstMessage(LoginPage::getUser().getId());
}

void MainWindow::sendFirstMessage(quint32 senderId)
{
    QDataStream stream(socket);
    stream << senderId << NULL << NULL;
}

/*QMap<quint32, Friend *> MainWindow::getFriendsMap()
{
    QMap<quint32, Friend *> friendsMap;
    QString friendsQuery = QString("SELECT %1_friends.id, "
                                          "%1_friends.username, "
                                          "%1_friends.alias, "
                                          "%1_friends.is_new_message, "
                                          "users.available "
                                          "FROM %1_friends "
                                          "JOIN users ON %1_friends.id = users.id")
                                   .arg(LoginPage::getUser().getId());

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(friendsQuery))
    {
        while (query.next())
        {
            friendsMap[query.value("id").toUInt()] = new Friend(query.value("id").toUInt(),
                                                                query.value("username").toString(),
                                                                query.value("alias").toString(),
                                                                query.value("is_new_message").toBool(),
                                                                query.value("available").toBool());
        }
    }

    qDebug() << "Jestem w metodzie getFriendsMap()";

    QMap<quint32, Friend *>::const_iterator iter;
    for (iter = friendsMap.constBegin(); iter != friendsMap.constEnd(); ++iter)
    {
        Friend *friendPtr = iter.value();

        qDebug() << "\t" << friendPtr->getUsername() << "- available: " << friendPtr->isAvailable();
    }

    return friendsMap;

}*/
QList<Friend *> MainWindow::getFriendsList()
{
    QList<Friend *> friends;
    QString friendsQuery = QString("SELECT %1_friends.id, "
                                          "%1_friends.username, "
                                          "%1_friends.alias, "
                                          "%1_friends.is_new_message, "
                                          "users.available "
                                          "FROM %1_friends "
                                          "JOIN users ON %1_friends.id = users.id")
                                   .arg(LoginPage::getUser().getId());

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(friendsQuery))
    {
        while (query.next())
        {
            friends.append(new Friend(query.value("id").toUInt(),
                                      query.value("username").toString(),
                                      query.value("alias").toString(),
                                      query.value("is_new_message").toBool(),
                                      query.value("available").toBool()));
        }
    }

    qDebug() << "Jestem w metodzie getFriendsList()";
    for (const Friend *friendPtr : friends)
    {
        qDebug() << "\t" << friendPtr->getUsername() << "- avaliable:" << friendPtr->isAvailable();
    }
    return friends;
}


/*void MainWindow::fillOutFriendsListWidget()
{
    QList<QListWidgetItem *> availableFriendsList;
    QList<QListWidgetItem *> unavailableFriendsList;
    QMap<quint32, Friend *>::const_iterator iter;

    for (iter = friendsMap.constBegin(); iter != friendsMap.constEnd(); ++iter)
    {
        Friend *friendPtr = iter.value();
        switch (friendPtr->getState())
        {
            case Friend::State::AvailableNoMessage:
                availableFriendsList.append(new QListWidgetItem(QIcon(":/images/available_icon.png"),
                                                                friendPtr->getAlias()));
                break;
            case Friend::State::AvailableWithMessage:
                availableFriendsList.append(new QListWidgetItem(QIcon(":/images/available_message_icon.png"),
                                                                friendPtr->getAlias()));
                break;
            case Friend::State::UnavailableNoMessage:
                unavailableFriendsList.append(new QListWidgetItem(QIcon(":/images/unavailable_icon.png"),
                                                                    friendPtr->getAlias()));
                break;
            case Friend::State::UnavailableWithMessage:
                unavailableFriendsList.append(new QListWidgetItem(QIcon(":/images/unavailable_message_icon.png"),
                                                                    friendPtr->getAlias()));
        }
    }

    std::sort(availableFriendsList.begin(), availableFriendsList.end(),
              [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); });
    std::sort(unavailableFriendsList.begin(), unavailableFriendsList.end(),
              [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); });

    for (QListWidgetItem* item : availableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << item->text();
    }

    for (QListWidgetItem* item : unavailableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << item->text();
    }

}*/

void MainWindow::fillOutFriendsListWidget()
{
    QList<QListWidgetItem *> availableFriendsList;
    QList<QListWidgetItem *> unavailableFriendsList;

    for (const Friend *friendPtr : friends)
    {
        switch (friendPtr->getState())
        {
            case Friend::State::AvailableNoMessage:
                availableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/available_icon.png"),
                                                                friendPtr->getAlias(), friendPtr->getId()));
                break;
            case Friend::State::AvailableWithMessage:
                availableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/available_message_icon.png"),
                                                                friendPtr->getAlias(), friendPtr->getId()));
                break;
            case Friend::State::UnavailableNoMessage:
                unavailableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/unavailable_icon.png"),
                                                                    friendPtr->getAlias(), friendPtr->getId()));
                break;
            case Friend::State::UnavailableWithMessage:
                unavailableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/unavailable_message_icon.png"),
                                                                    friendPtr->getAlias(), friendPtr->getId()));
        }
    }

    std::sort(availableFriendsList.begin(), availableFriendsList.end(),
              [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); });
    std::sort(unavailableFriendsList.begin(), unavailableFriendsList.end(),
              [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); });

    // ONLY TO DEBUG
    for (QListWidgetItem* item : availableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << item->text();
    }

    for (QListWidgetItem* item : unavailableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << item->text();
    }
}

/*void MainWindow::fillOutFriendsListWidget()
{
    QList<QListWidgetItem *> availableFriendsList;
    QList<QListWidgetItem *> unavailableFriendsList;

    for (const Friend *friendPtr : friends)
    {
        switch (friendPtr->getState())
        {
            case Friend::State::AvailableNoMessage:
                availableFriendsList.append(new QListWidgetItem(QIcon(":/images/available_icon.png"),
                                                                friendPtr->getAlias()));
                break;
            case Friend::State::AvailableWithMessage:
                availableFriendsList.append(new QListWidgetItem(QIcon(":/images/available_message_icon.png"),
                                                                friendPtr->getAlias()));
                break;
            case Friend::State::UnavailableNoMessage:
                unavailableFriendsList.append(new QListWidgetItem(QIcon(":/images/unavailable_icon.png"),
                                                                    friendPtr->getAlias()));
                break;
            case Friend::State::UnavailableWithMessage:
                unavailableFriendsList.append(new QListWidgetItem(QIcon(":/images/unavailable_message_icon.png"),
                                                                    friendPtr->getAlias()));
        }
    }

    std::sort(availableFriendsList.begin(), availableFriendsList.end(),
              [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); });
    std::sort(unavailableFriendsList.begin(), unavailableFriendsList.end(),
              [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); });

    // ONLY TO DEBUG
    for (QListWidgetItem* item : availableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << item->text();
    }

    for (QListWidgetItem* item : unavailableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << item->text();
    }
}*/

void MainWindow::reloadFriendsListWidget()
{
    ui->friendsListWidget->clear();
    fillOutFriendsListWidget();
}

void MainWindow::makeThread()
{
    friendsStatuses = new FriendsStatuses();
    QThread *thread = new QThread;
    friendsStatuses->moveToThread(thread);

    QObject::connect(thread, &QThread::started, friendsStatuses, &FriendsStatuses::run);
    //QObject::connect(friendsStatuses, &FriendsStatuses::statusChanged, this, &MainWindow::updateFriendsList);
    QObject::connect(friendsStatuses, &FriendsStatuses::availabilityStatusChanged, this, &MainWindow::changeAvailabilityStatus);
    QObject::connect(friendsStatuses, &FriendsStatuses::messageStatusChanged, this, &MainWindow::changeMessageStatus);
    // NIE WIEM JAKA JEST NAZWA this DLATEGO ZROBIŁEM TO W TYM PLIKU, A NIE W friensstatuses.cpp (mainWindow?)

    thread->start();
}




void MainWindow::createFriendsList()
{
    Queries queries;
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(queries.availableFriendsQuery))
    {
        while (query.next())
        {
            QListWidgetItem *item = new QListWidgetItem;

            if (query.value("is_new_message").toBool() == true)
            {
                if (!activeWindowsList.contains(query.value("id").toUInt()))
                {
                    addFriendToList(item, query.value("username_alias").toString(),
                                    QIcon(":/images/available_message_icon.png"));
                }
                else //////// ++++++++
                    addFriendToList(item, query.value("username_alias").toString(),
                                    QIcon(":/images/available_icon.png"));
            }
            else
            {
                addFriendToList(item, query.value("username_alias").toString(),
                                QIcon(":/images/available_icon.png"));
            }
        }
    }
    if (query.exec(queries.unavailableFriendsQuery))
    {
        while (query.next())
        {
            QListWidgetItem *item = new QListWidgetItem;

            if (query.value("is_new_message").toBool() == true)
            {
                if (!activeWindowsList.contains(query.value("id").toUInt()))
                {
                    addFriendToList(item, query.value("username_alias").toString(),
                                    QIcon(":/images/unavailable_message_icon.png"));
                }
            }
            else
            {
                addFriendToList(item, query.value("username_alias").toString(),
                                QIcon(":/images/unavailable_icon.png"));
            }
        }
    }
}

void MainWindow::addFriendToList(QListWidgetItem *item, QString friendUsername, QIcon icon)
{
    item->setText(friendUsername);
    item->setIcon(icon);
    ui->friendsListWidget->addItem(item);
}


void MainWindow::updateFriendsList(quint32, bool)
{

}

/*void MainWindow::changeAvailabilityStatus(quint32 id, bool available)
{
    qDebug() << "zmiana dostępności uzytkownika " << id << available;

    QMap<quint32, Friend *>::iterator iter;
    for (iter = friendsMap.begin(); iter != friendsMap.end(); ++iter)
    {
        Friend *friendPtr = iter.value();

        qDebug() << "\t" << friendPtr->getUsername() << "- available: " << friendPtr->isAvailable();
    }



    for (Friend* friendPtr : friends)
    {
        if ((*friendPtr)() == id)
            friendPtr->setAvailable(available);
    }
    reloadFriendsListWidget();
}*/

void MainWindow::changeAvailabilityStatus(quint32 id, bool available)
{
    qDebug() << "zmiana dostępności uzytkownika " << id << available;
    for (Friend* friendPtr : friends)
    {
        if ((*friendPtr)() == id)
            friendPtr->setAvailable(available);
    }
    reloadFriendsListWidget();

}

void MainWindow::changeMessageStatus(quint32 id, bool newMessage)
{
    qDebug() << "zmiana statusu wiadomosci uzytkownika" << id << newMessage;
    for (Friend* friendPtr : friends)
    {
        if ((*friendPtr)() == id)
            friendPtr->setNewMessage(newMessage);
    }
    reloadFriendsListWidget();
}

/*void MainWindow::updateFriendsList()
{
    qDebug() << "updateFriendsList()";
    ui->friendsListWidget->clear();
    createFriendsList();
}*/



/*void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    quint32 friendId = getFriendIdFromItem(item);
    if (!activeWindowsList.contains(friendId))
    {
        activeWindowsList.push_back(friendId);
        ChatWindow *chatWindow = new ChatWindow(friendId, this);
        chatWindow->show();
        if (checkForNewMessage(friendId))
        {
            changeNewMessageState(friendId, UNAVAILABLE);
            //updateFriendsList();
            reloadFriendsListWidget();
        }
    }
}*/

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ExtendedQListWidgetItem *extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(item);
    QString friendAlias = extendedItem->text();
    quint32 friendId = extendedItem->getId();

    qDebug() << "doubleClick" << extendedItem->getId();

    if (!activeWindowsList.contains(friendId))
    {
        activeWindowsList.push_back(friendId);
        ChatWindow *chatWindow = new ChatWindow(friendId, this);
        chatWindow->show();
        if (checkForNewMessage(friendId))
        {
            changeNewMessageState(friendId, UNAVAILABLE);
            reloadFriendsListWidget();
        }
    }
}

quint32 MainWindow::getFriendIdFromItem(QListWidgetItem *item)
{
    QString sqlCommand = "SELECT " + LoginPage::getUser().getUsername() + "_friends.id FROM " + LoginPage::getUser().getUsername() + "_friends " +
            "WHERE username_alias = " + '"' + item->text() + '"';
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(sqlCommand))
    {
        if (query.size() > 0)
        {
            query.next();
            return query.value("id").toUInt();
        }
    }

    return 0;
}

bool MainWindow::checkForNewMessage(quint32 userId)
{
    QString sqlCommand = "SELECT * FROM " +
            QString::number(LoginPage::getUser().getId()) + "_friends WHERE " +
            LoginPage::getUser().getUsername() + "_friends.id = " + "'" +
            QString::number(userId) + "'";

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(sqlCommand))
    {
        if (query.size() > 0)
        {
            query.next();
            return query.value("is_new_message").toUInt();
        }
    }

    return 0;
}

void MainWindow::changeNewMessageState(quint32 userId, quint32 state)
{
    QString sqlCommand = "UPDATE " + LoginPage::getUser().getUsername() + "_friends SET " +
                LoginPage::getUser().getUsername() + "_friends.is_new_message = " + QString::number(state) + " WHERE " +
                LoginPage::getUser().getUsername() + "_friends.id = " + "'" + QString::number(userId) + "'";

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(sqlCommand))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "Updated is_new_message state for user" << userId;
        }
        else
        {
            qDebug() << "Update is_new_message state for user" << userId << "failed.";
        }
    }
}


void MainWindow::socketDisconnected()
{
    qDebug() << "Disconnected from server.";
}



