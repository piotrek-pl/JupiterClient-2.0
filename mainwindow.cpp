#include <QThread>
#include <QHostAddress>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "friendsstatuses.h"
#include "loginpage.h"
#include "chatwindow.h"
#include "extendedqlistwidgetitem.h"

QList<quint32> MainWindow::activeWindowsList;
QTcpSocket *MainWindow::socket;
QList<Friend *> MainWindow::friends;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectToServer();
    friends = getFriendsList();
    fillOutFriendsListWidget();
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
            case Friend::State::AvailableWithMessageOpenChatWindow:
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
                break;
            case Friend::State::UnavailableWithMessageOpenChatWindow:
                unavailableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/unavailable_icon.png"),
                                                                friendPtr->getAlias(), friendPtr->getId()));
        }
    }

    auto compareItems = [](QListWidgetItem* item1, QListWidgetItem* item2) { return item1->text() < item2->text(); };
    std::sort(availableFriendsList.begin(), availableFriendsList.end(), compareItems);
    std::sort(unavailableFriendsList.begin(), unavailableFriendsList.end(), compareItems);

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
    qDebug() << "MainWindow::fillOutFriendsListWidget() - wykonało się";
}

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
    QObject::connect(friendsStatuses, &FriendsStatuses::availabilityStatusChanged, this, &MainWindow::changeAvailabilityStatus);
    QObject::connect(friendsStatuses, &FriendsStatuses::messageStatusChanged, this, &MainWindow::changeFriendMessageStatus);
    // NIE WIEM JAKA JEST NAZWA this DLATEGO ZROBIŁEM TO W TYM PLIKU, A NIE W friensstatuses.cpp (mainWindow?)

    thread->start();
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

void MainWindow::changeFriendMessageStatus(quint32 id, bool newMessage)
{
    qDebug() << "MainWindow::changeMessageStatus() - wykonało się";
    if (activeWindowsList.contains(id))
    {
        qDebug() << "okno" << id << "aktywne";


        // wyswietl cos na oknie chatu
        ///////////////////////////////
        // jesli okno jest aktywne w sensie, ze nie pracuje w tle
        // oznacz wiadomość jako przeczytaną
        // tj. changeMessageStateToRead(id);
        // w innym razie niech okienko zacznie migać
        //

        for (Friend* friendPtr : friends)
        {
            if ((*friendPtr)() == id)
            {
                qDebug() << "zmieniam stan" << id << "na" << newMessage;
                qDebug() << "zmieniam openChatWindow na true";
                friendPtr->setNewMessage(newMessage);
                friendPtr->setOpenChatWindow(true);
                qDebug() << "stan wiadomosci przyjaciela to:";
                qDebug() << friendPtr->isNewMessage();
                qDebug() << "stan otwartego okna to:";
                qDebug() << friendPtr->isOpenChatWindow();
            }
        }

        for (Friend* friendPtr : friends)
        {
            qDebug() << friendPtr->getId() << friendPtr->isNewMessage();
            if (!friendPtr->isNewMessage() && friendPtr->isOpenChatWindow())
                reloadFriendsListWidget();

        }
        //reloadFriendsListWidget();

        return;
    }
    qDebug() << "MainWindow::changeMessageStatus() - zmiana statusu wiadomosci uzytkownika" << id << "na wartosc" << newMessage;

    for (Friend* friendPtr : friends)
    {
        if ((*friendPtr)() == id)
        {
            qDebug() << "zmieniam stan" << id << "na" << newMessage;
            friendPtr->setNewMessage(newMessage);
            qDebug() << "stan wiadomosci przyjaciela to:";
            qDebug() << friendPtr->isNewMessage();

            if (friendPtr->isNewMessage() && !friendPtr->isOpenChatWindow())
                reloadFriendsListWidget();
        }
    }
    //reloadFriendsListWidget();
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ExtendedQListWidgetItem *extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(item);
    QString friendAlias = extendedItem->text();
    quint32 friendId = extendedItem->getId();

    qDebug() << "MainWindow::on_listWidget_itemDoubleClicked() - on user:" << extendedItem->getId();

    if (!activeWindowsList.contains(friendId))
    {
        if (isNewMessage(friendId))
            changeMessageStatusInTheDatabaseToRead(friendId);

        //changeMessageStatusInTheDatabaseToRead(friendId);

        activeWindowsList.push_back(friendId);
        qDebug() << "activeWindowList.push_back(" << friendId << ")";

        //ChatWindow *chatWindow = new ChatWindow(friendId, this);
        //chatWindow->show();
        chatWindowsMap.insert(friendId, new ChatWindow(friendId, this));
        ChatWindow *chatWindow = chatWindowsMap.value(friendId);
        chatWindowsMap.value(friendId)->show();



        /*if (!isNewMessage(friendId)) // nie działa sprawdzanie czy jest wiadomosc
        {
            qDebug() << "if pokazuje:" << isNewMessage(friendId);
            changeMessageStatusToRead(friendId);
            //reloadFriendsListWidget();
        }*/
        /*for (Friend* friendPtr : friends)
        {
            qDebug() << friendPtr->getId() << friendPtr->isNewMessage();
        }*/
        //changeMessageStatusToRead(friendId);
    }
}

bool MainWindow::isNewMessage(quint32 friendId)
{
    qDebug() << "jestem w isNewMessage()";
    QList<Friend*>::iterator result = std::find_if(friends.begin(), friends.end(), [friendId](Friend* friendPtr)
    {
        qDebug() << friendPtr->getId();
        qDebug() << friendId;
        return friendPtr->getId() == friendId;
    });
    for (Friend* friendPtr : friends)
    {
        qDebug() << friendPtr->getId() << friendPtr->isNewMessage();
    }

    Friend* foundFriend = *result;
    qDebug() << foundFriend->isNewMessage();

    if (foundFriend->isNewMessage())
        return true;

    return false;
}

void MainWindow::changeMessageStatusInTheDatabaseToRead(quint32 friendId)
{
    //*QString updateNewMessageState = QString("UPDATE %1_friends SET %1_friends.is_new_message = '0' WHERE %1_friends.id = '%3'")
      //                                      .arg(LoginPage::getUser().getId(), /*false*/ friendId);

    QString updateNewMessageState = "UPDATE " + QString::number(LoginPage::getUser().getId()) + "_friends SET " +
                QString::number(LoginPage::getUser().getId()) + "_friends.is_new_message = " + QString::number(0) + " WHERE " +
                QString::number(LoginPage::getUser().getId()) + "_friends.id = " + "'" + QString::number(friendId) + "'";

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(updateNewMessageState))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "MainWindow::changeMessageStatusToRead() - wykonało się";
            qDebug() << "Updated is_new_message state for user" << friendId;
        }
        else
        {
            qDebug() << "MainWindow::changeMessageStatusToRead() - wykonało się";
            qDebug() << "Update is_new_message state for user" << friendId << "failed.";
        }
    }

    for (Friend* friendPtr : friends)
    {
        qDebug() << friendPtr->getId() << friendPtr->isNewMessage();
    }
}

/*void MainWindow::changeNewMessageState(quint32 userId, quint32 state)
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
}*/



/*bool MainWindow::checkForNewMessage(quint32 userId)
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
}*/




void MainWindow::socketDisconnected()
{
    qDebug() << "Disconnected from server.";
}



