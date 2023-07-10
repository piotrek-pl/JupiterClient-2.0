#include <QThread>
#include <QHostAddress>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "friendsstatuses.h"
#include "loginpage.h"
#include "chatwindow.h"
#include "extendedqlistwidgetitem.h"

QList<quint32> MainWindow::activeWindowsList;
QMap<quint32, ChatWindow *> MainWindow::activeChatWindowsMap;
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
    qDebug() << "Jestem w metodzie getFriendsList()";

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

    for (const Friend *friendPtr : friends)
    {
        qDebug() << "\t" << friendPtr->getUsername() << "- avaliable:" << friendPtr->isAvailable();
    }
    return friends;
}

void MainWindow::fillOutFriendsListWidget()
{
    qDebug() << "Jestem w metodzie fillOutFriendsListWidget()";

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
                availableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/available_icon.png"),
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
        qDebug() << "\t" << item->text();
    }

    for (QListWidgetItem* item : unavailableFriendsList)
    {
        ui->friendsListWidget->addItem(item);
        qDebug() << "\t" << item->text();
    }
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
    qDebug() << "Jestem w metodzie changeAvailabilityStatus()";
    qDebug() << "\tZmiana dostępności uzytkownika" << id << "na status" << available;
    for (Friend* friendPtr : friends)
    {
        if ((*friendPtr)() == id)
            friendPtr->setAvailable(available);
    }
    reloadFriendsListWidget();

}

void MainWindow::changeFriendMessageStatus(quint32 id, bool newMessage)
{
    qDebug() << "Jestem w metodzie changeFriendMessageStatus()";

//for (Friend* friendPtr : friends)
//{
    //if (friendPtr->getId() == id)
    //if (friendPtr->isOpenChatWindow())


    if (activeChatWindowsMap.contains(id))
    {
        qDebug() << "\tOkno" << id << "aktywne";

        /*ChatWindow *chatWindow = activeChatWindowsMap.value(id);
        if (!isNewMessage(id))
            chatWindow->setWindowIcon(QIcon(":/images/available_message_icon.png"));*/

        for (Friend* friendPtr : friends)
        {
            if ((*friendPtr)() == id)
            {
                qDebug() << "\tZmiana stanu wiadomości użytkownika" << id << "na stan" << newMessage;
                friendPtr->setNewMessage(newMessage);

                //qDebug() << "\tZmiana składowej openChatWindow na true";
                //friendPtr->setOpenChatWindow(true);
                if (!friendPtr->isNewMessage())
                    reloadFriendsListWidget();
            }
        }
        //reloadFriendsListWidget();

        /*for (Friend* friendPtr : friends)
        {
            if (!friendPtr->isNewMessage() && friendPtr->isOpenChatWindow()
                && !activeChatWindowsMap.value(id)->windowIcon().isNull())
                // tutaj przegiąłem, sprawdzanie czy jest ustawiona ikona jest zdradliwe
                reloadFriendsListWidget();

        }*/

        return;
    }
//}
    qDebug() << "\tOkno" << id << "nieaktywne";


    for (Friend* friendPtr : friends)
    {
        if ((*friendPtr)() == id)
        {
            qDebug() << "\tZmiana stanu wiadomości użytkownika" << id << "na stan" << newMessage;
            friendPtr->setNewMessage(newMessage);
            //qDebug() << "\tZmiana składowej openChatWindow na false";
            //friendPtr->setOpenChatWindow(false);

            reloadFriendsListWidget();

            /*if (friendPtr->isNewMessage() && !friendPtr->isOpenChatWindow())
                reloadFriendsListWidget();*/
        }
    }
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ExtendedQListWidgetItem *extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(item);
    QString friendAlias = extendedItem->text();
    quint32 friendId = extendedItem->getId();

    qDebug() << "Jestem w metodzie on_listWidget_itemDoubleClicked() - klinięto na użytkownika:" << extendedItem->getId();

    if (!activeChatWindowsMap.value(friendId))
    {
        qDebug() << "\tOkno chatu" << friendId << "nieaktywne";
        qDebug() << "\tSprawdzam czy jest nowa wiadomość - wywołuję isNewMessage()";
        if (isNewMessage(friendId))
            changeMessageStatusInTheDatabaseToRead(friendId);

        activeChatWindowsMap.insert(friendId, new ChatWindow(friendId, this));
        activeChatWindowsMap.value(friendId)->show();

        for (Friend* friendPtr : friends)
        {
            if ((*friendPtr)() == friendId)
            {
                qDebug() << "\tZmiana składowej openChatWindow na true";
                friendPtr->setOpenChatWindow(true);
            }
        }

    }
    else
        qDebug() << "\tOkno chatu" << friendId << "aktywne";
}

bool MainWindow::isNewMessage(quint32 friendId)
{
    qDebug() << "Jestem w metodzie isNewMessage() - START";
    QList<Friend*>::iterator result = std::find_if(friends.begin(), friends.end(), [friendId](Friend* friendPtr)
    {
        return friendPtr->getId() == friendId;
    });

    for (Friend* friendPtr : friends)
    {
        qDebug() << "\tUżytkownik" << friendPtr->getId() << "- isNewMessage() zwraca:" << friendPtr->isNewMessage();
    }

    Friend* foundFriend = *result;
    qDebug() << "\tfoundFriend->isNewMessage() zwraca" << foundFriend->isNewMessage();

    if (foundFriend->isNewMessage())
    {
        qDebug() << "Jestem w metodzie isNewMessage() - KONIEC";
        return true;
    }

    qDebug() << "Jestem w metodzie isNewMessage() - KONIEC";
    return false;
}

void MainWindow::changeMessageStatusInTheDatabaseToRead(quint32 friendId)
{
    //*QString updateNewMessageState = QString("UPDATE %1_friends SET %1_friends.is_new_message = '0' WHERE %1_friends.id = '%3'")
    //                                      .arg(LoginPage::getUser().getId(), /*false*/ friendId);

    qDebug() << "Jestem w metodzie changeMessageStatusInTheDatabaseToRead()";

    QString updateNewMessageState = "UPDATE " + QString::number(LoginPage::getUser().getId()) + "_friends SET " +
                QString::number(LoginPage::getUser().getId()) + "_friends.is_new_message = " + QString::number(0) + " WHERE " +
                QString::number(LoginPage::getUser().getId()) + "_friends.id = " + "'" + QString::number(friendId) + "'";

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(updateNewMessageState))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tZmieniono stan is_new_message dla użytkownika" << friendId << "na wartosc false";
        }
        else
        {
            qDebug() << "\tNieudana zmiana stanu is_new_message dla użytkownika" << friendId;
        }
    }

    /*for (Friend* friendPtr : friends)
    {
        qDebug() << "\tUżytkownik" <<friendPtr->getId() << "isNewMessage() zwraca:" << friendPtr->isNewMessage();
    }*/
}

void MainWindow::socketDisconnected()
{
    qDebug() << "Disconnected from server.";
}



