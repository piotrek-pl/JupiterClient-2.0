#include <QThread>
#include <QHostAddress>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "friendsstatuses.h"
#include "loginpage.h"
#include "chatwindow.h"
#include "extendedqlistwidgetitem.h"

QMap<quint32, ChatWindow *> MainWindow::activeChatWindowsMap;
QTcpSocket *MainWindow::socket;
QMap<quint32, Friend* > MainWindow::friendsMap;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectToServer();
    friendsMap = getFriendsMap();
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

QMap<quint32, Friend *> MainWindow::getFriendsMap()
{
    qDebug() << "Jestem w metodzie getFriendsMap()";

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
            friendsMap.insert(query.value("id").toUInt(),
                                new Friend(query.value("id").toUInt(),
                                           query.value("username").toString(),
                                           query.value("alias").toString(),
                                           query.value("is_new_message").toBool(),
                                           query.value("available").toBool()));
        }
    }

    for (const auto& id : friendsMap.keys())
    {
        qDebug() << "\t" << friendsMap.value(id)->getUsername() << "- available: "
                 << friendsMap.value(id)->isAvailable();
    }

    return friendsMap;
}

void MainWindow::fillOutFriendsListWidget()
{
    qDebug() << "Jestem w metodzie fillOutFriendsListWidget()";

    QList<QListWidgetItem *> availableFriendsList;
    QList<QListWidgetItem *> unavailableFriendsList;

    for (const auto& id : friendsMap.keys())
    {
        switch (friendsMap.value(id)->getState())
        {
            case Friend::State::AvailableNoMessage:
                availableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/available_icon.png"),
                                                                friendsMap.value(id)->getAlias(), id));
                break;
            case Friend::State::AvailableWithMessage:
                availableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/available_message_icon.png"),
                                                                friendsMap.value(id)->getAlias(), id));
                break;
            case Friend::State::AvailableWithMessageOpenChatWindow:
                availableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/available_icon.png"),
                                                            friendsMap.value(id)->getAlias(), id));

                break;
            case Friend::State::UnavailableNoMessage:
                unavailableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/unavailable_icon.png"),
                                                                    friendsMap.value(id)->getAlias(), id));
                break;
            case Friend::State::UnavailableWithMessage:
                unavailableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/unavailable_message_icon.png"),
                                                                    friendsMap.value(id)->getAlias(), id));
                break;
            case Friend::State::UnavailableWithMessageOpenChatWindow:
                unavailableFriendsList.append(new ExtendedQListWidgetItem(QIcon(":/images/unavailable_icon.png"),
                                                                friendsMap.value(id)->getAlias(), id));
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

void MainWindow::changeAvailabilityStatus(quint32 id, bool available)
{
    qDebug() << "Jestem w metodzie changeAvailabilityStatus()";
    qDebug() << "\tZmiana dostępności uzytkownika" << id << "na status" << available;

    friendsMap.value(id)->setAvailable(available);
    reloadFriendsListWidget();
}

void MainWindow::changeFriendMessageStatus(quint32 id, bool newMessage)
{
    qDebug() << "Jestem w metodzie changeFriendMessageStatus()";

    if (friendsMap.value(id)->isOpenChatWindow())
    {
        qDebug() << "\tOkno" << id << "aktywne";
        qDebug() << "\tZmiana stanu wiadomości użytkownika" << id << "na stan" << newMessage;
        qDebug() << "\tfriendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage() -" << friendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage();
        if (friendsMap.value(id)->isNewMessage() && !friendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage())
        {
            reloadFriendsListWidget();
            qDebug() << "\tZmiana friendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage() na wartość true";
            friendsMap.value(id)->setChatWindowOpenedAfterReceivingTheMessage(true);
        }
    }
    else
    {
        qDebug() << "\tOkno" << id << "nieaktywne";
        qDebug() << "\tZmiana stanu wiadomości użytkownika" << id << "na stan" << newMessage;
        friendsMap.value(id)->setNewMessage(newMessage);
        reloadFriendsListWidget();
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

        qDebug() << "\tZmiana składowej openChatWindow na true";
        friendsMap.value(friendId)->setOpenChatWindow(true);
    }
    else
        qDebug() << "\tOkno chatu" << friendId << "aktywne";
}

bool MainWindow::isNewMessage(quint32 friendId)
{


    qDebug() << "Jestem w metodzie isNewMessage() - START";
    for (const auto& id : friendsMap.keys())
    {
        qDebug() << "\tUżytkownik" << friendsMap.value(id)->getId()
                 << "- isNewMessage() zwraca:" << friendsMap.value(id)->isNewMessage();
    }

    if (friendsMap.value(friendId)->isNewMessage())
    {
        qDebug() << "Jestem w metodzie isNewMessage() - KONIEC";
        return true;
    }
    else
    {
        qDebug() << "Jestem w metodzie isNewMessage() - KONIEC";
        return false;
    }
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
}

void MainWindow::socketDisconnected()
{
    qDebug() << "Disconnected from server.";
}



