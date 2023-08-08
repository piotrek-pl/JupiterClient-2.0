#include <QThread>
#include <QHostAddress>
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "friendsstatuses.h"
#include "loginpage.h"
#include "chatwindow.h"
#include "extendedqlistwidgetitem.h"

QMap<quint32, ChatWindow *> MainWindow::activeChatWindowsMap;
QTcpSocket *MainWindow::socket;
QMap<quint32, Friend* > MainWindow::friendsMap;

QList<Invitation *> MainWindow::sentInvitationsList;
QList<Invitation *> MainWindow::receivedInvitationsList;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QString("%1 [id: %2]")
                   .arg(LoginPage::getUser().getUsername(),
                        QString::number(LoginPage::getUser().getId())));

    connectToServer();
    friendsMap = getFriendsMap();
    sentInvitationsList = getInvitationsList("sent");
    receivedInvitationsList = getInvitationsList("received");

    fillOutFriendsListWidget();
    makeThreads();

    QObject::connect(ui->friendsListWidget, &QListWidget::customContextMenuRequested,
                     this, &MainWindow::handleFriendsListWidgetContextMenu);
    QObject::connect(ui->actionIInvited, &QAction::triggered,
                     this, &MainWindow::onActionIInvitedClicked);
    QObject::connect(ui->actionInvitedMe, &QAction::triggered,
                     this, &MainWindow::onActionInvitedMeClicked);

    /*QObject::connect(iInvitedListWidget, &QListWidget::customContextMenuRequested,
                     this, &MainWindow::handleIInvitedListWidgetContextMenu);
    QObject::connect(invitedMeListWidget, &QListWidget::customContextMenuRequested,
                     this, &MainWindow::handleInvitedMeListWidgetContextMenu);*/

    setWindowIcon(QIcon(":/images/jupiter_icon.png"));
    //setWindowFlags(windowFlags() &(~Qt::WindowMaximizeButtonHint));
}

void MainWindow::handleFriendsListWidgetContextMenu(const QPoint &pos)
{
    qDebug() << "Jestem w metodzie handleFriendsListWidgetContextMenu(const QPoint &pos)";

    QListWidgetItem* clickedItem = ui->friendsListWidget->itemAt(pos);

    if (clickedItem == nullptr)
        return;

    ExtendedQListWidgetItem* extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(clickedItem);
    quint32 friendId = extendedItem->getId();

    QPoint item = ui->friendsListWidget->mapToGlobal(pos);
    QMenu submenu;
    submenu.addAction("Rename");
    submenu.addAction("Delete");
    QAction *rightClickItem = submenu.exec(item);

    if (rightClickItem && rightClickItem->text().contains("Rename"))
    {
        qDebug() << "\tRename - id:" << friendId;
        bool ok;
        QString newAlias = QInputDialog::getText(this, "Rename Friend", "Enter new alias:", QLineEdit::Normal, "", &ok);
        if (ok && !newAlias.isEmpty())
        {
            if (changeUsernameAliasInTheDatabase(newAlias, friendId))
            {
                friendsMap.value(friendId)->setAlias(newAlias);
                reloadFriendsListWidget();
            }
        }
        else
        {

        }
    }
    else if (rightClickItem && rightClickItem->text().contains("Delete"))
    {
        qDebug() << "\tDelete - id:" << friendId;

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation", "Are you sure to remove?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            if (removeFriendFromDatabase(LoginPage::getUser().getId(), friendId))
            {
                if (activeChatWindowsMap.contains(friendId))
                {
                    activeChatWindowsMap.value(friendId)->close();
                }
                friendsMap.remove(friendId);
                reloadFriendsListWidget();
                removeFriendFromDatabase(friendId, LoginPage::getUser().getId());
                deleteChatTable(LoginPage::getUser().getId(), friendId);
                deleteChatTable(friendId, LoginPage::getUser().getId());
            }
        }
    }
}

void MainWindow::handleIInvitedListWidgetContextMenu(const QPoint &pos)
{
    qDebug() << "Jestem w handleIInvitedListWidgetContextMenu(const QPoint &pos)";

    QListWidgetItem* clickedItem = iInvitedListWidget->itemAt(pos);

    if (clickedItem == nullptr)
        return;

    ExtendedQListWidgetItem* extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(clickedItem);
    quint32 userId = extendedItem->getId();

    QPoint item = iInvitedListWidget->mapToGlobal(pos);
    QMenu submenu;
    submenu.addAction("Cancel");
    QAction *rightClickItem = submenu.exec(item);

    if (rightClickItem && rightClickItem->text().contains("Cancel"))
    {
        qDebug() << "\tCancel - id:" << userId;
        removeInvitationFromDatabase(userId, "sent");
        removeInvitationFromAnotherUsersTable(userId, "received");
    }

}

void MainWindow::handleInvitedMeListWidgetContextMenu(const QPoint &pos)
{
    qDebug() << "Jestem w handleInvitedMeListWidgetContextMenu(const QPoint &pos)";

    QListWidgetItem* clickedItem = invitedMeListWidget->itemAt(pos);

    if (clickedItem == nullptr)
        return;

    ExtendedQListWidgetItem* extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(clickedItem);
    quint32 userId = extendedItem->getId();

    QPoint item = invitedMeListWidget->mapToGlobal(pos);
    QMenu submenu;
    submenu.addAction("Accept");
    submenu.addAction("Decline");
    QAction *rightClickItem = submenu.exec(item);

    if (rightClickItem && rightClickItem->text().contains("Accept"))
    {
        qDebug() << "\tAccept - id:" << userId;
        addFriendToDatabase(LoginPage::getUser().getId(), userId);
        addChatTableToDatabase(LoginPage::getUser().getId(), userId);
        removeInvitationFromDatabase(userId, "received");
        addFriendToDatabase(userId, LoginPage::getUser().getId());
        addChatTableToDatabase(userId, LoginPage::getUser().getId());
        removeInvitationFromAnotherUsersTable(userId, "sent");
    }
    else if (rightClickItem && rightClickItem->text().contains("Decline"))
    {
        qDebug() << "\tDecline - id:" << userId;
        removeInvitationFromDatabase(userId, "received");
        removeInvitationFromAnotherUsersTable(userId, "sent");
    }
}

void MainWindow::handleInviteAction(quint32 userId)
{
    if (userId == LoginPage::getUser().getId())
    {
        QMessageBox::warning(this, "Error", "You can't invite yourself");
        return;
    }

    for (const auto& key : friendsMap.keys())
    {
        if (key == userId)
        {
            QMessageBox::warning(this, "Friend already on list", "The user is already on your friends list.");
            return;
        }

    }

    for (const auto& item : sentInvitationsList)
    {
        if (item->getUserId() == userId)
        {
            QMessageBox::warning(this, "User already invited", "This user has already been invited.");
            return;
        }
    }

    if (inviteUserToFriends(userId))
    {
        QMessageBox::information(this, "Information", "An invitation has been sent to a user.");
    }
    else
    {
        QMessageBox::critical(this, "Error", "An error occurred while inviting the user.");
    }

}

bool MainWindow::removeFriendFromDatabase(quint32 userId, quint32 friendId)
{
    qDebug() << "Jestem w metodzie removeFriendFromDatabase(quint32 friendId)";
    QString deleteFriend = QString("DELETE FROM %1_friends WHERE id = '%2'")
                               .arg(userId)
                               .arg(friendId);
    qDebug() << "\t" << deleteFriend;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(deleteFriend))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tUsunięto z tabeli użytkownika" <<userId
                     << "znajomego" << friendId;
            return true;
        }
        else
        {
            qDebug() << "\tBłąd podczas usuwania znajomego" << friendId
                     << "z tabeli użytkownika" << userId;
        }
    }
    return false;
}

bool MainWindow::changeUsernameAliasInTheDatabase(QString newAlias, quint32 friendId)
{
    qDebug() << "Jestem w metodzie changeUsernameAliasInTheDatabase(...)";
    QString updateUsernameAlias = QString("UPDATE %1_friends SET %1_friends.alias = '%2' WHERE %1_friends.id = '%3'")
                                      .arg(LoginPage::getUser().getId()).arg(newAlias).arg(friendId);
    qDebug() << "\t" << updateUsernameAlias;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(updateUsernameAlias))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tZmieniono alias dla użytkownika" << friendId << "na wartosc" << newAlias;
            return true;
        }
        else
        {
            qDebug() << "\tNieudana zmiana aliasu dla użytkownika" << friendId;
        }
    }
    return false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::connectToServer()
{
    socket = new QTcpSocket();

    connect(socket, &QTcpSocket::connected, this, &MainWindow::socketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::socketDisconnected);
    socket->connectToHost("77.237.31.25", 1234);

    if (socket->waitForConnected())
    {
        qDebug() << "Connected to the server!";
        return true;
    }
    else
    {
        qDebug() << "Failed to connect to the server!";
        return false;
    }

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
    stream << senderId << 0 << 0;
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




void MainWindow::makeThreads()
{
    friendsStatuses = new FriendsStatuses();
    QThread *firstThread = new QThread;
    friendsStatuses->moveToThread(firstThread);

    QObject::connect(firstThread, &QThread::started, friendsStatuses, &FriendsStatuses::run);
    QObject::connect(friendsStatuses, &FriendsStatuses::availabilityStatusChanged, this, &MainWindow::changeAvailabilityStatus);
    QObject::connect(friendsStatuses, &FriendsStatuses::messageStatusChanged, this, &MainWindow::changeFriendMessageStatus);
    QObject::connect(friendsStatuses, &FriendsStatuses::removedFriend, this, &MainWindow::onRemovedFriend);
    QObject::connect(friendsStatuses, &FriendsStatuses::addedFriend, this, &MainWindow::onAddedFriend);

    firstThread->start();

    invitationController = new InvitationController();
    QThread *secondThread = new QThread;
    invitationController->moveToThread(secondThread);

    QObject::connect(secondThread, &QThread::started, invitationController, &InvitationController::run);
    QObject::connect(invitationController, &InvitationController::sentInvitationsChanged, this, &MainWindow::changeSentInvitationList);
    QObject::connect(invitationController, &InvitationController::receivedInvitationsChanged, this, &MainWindow::changeReceivedInvitationList);

    secondThread->start();



}

void MainWindow::changeAvailabilityStatus(quint32 id, bool available)
{
    /// BEDZIE TRZEBA SPRAWDZIC CZY UZYTKOWNIK ISTNIEJE
    /// W PRZYPADKU DODAWANIA UZYTKOWNIKA
    /// BO PROGRAM SIE CRASHUJE
    qDebug() << "Jestem w metodzie changeAvailabilityStatus()";
    qDebug() << "\tZmiana dostępności uzytkownika" << id << "na status" << available;

    friendsMap.value(id)->setAvailable(available);
    reloadFriendsListWidget();
}

void MainWindow::changeFriendMessageStatus(quint32 id, bool newMessage)
{
    qDebug() << "Jestem w metodzie changeFriendMessageStatus()";
    qDebug() << "\targument newMessage =" << newMessage;

    if (friendsMap.value(id)->isOpenChatWindow())
    {


        qDebug() << "\tOkno" << id << "aktywne";
        qDebug() << "\tfriendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage() -"
                 << friendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage();


        if (!friendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage())
        {
            reloadFriendsListWidget();
            qDebug() << "\tZmiana friendsMap.value(id)->wheterChatWindowOpenedAfterReceivingTheMessage() na wartość true";
            friendsMap.value(id)->setChatWindowOpenedAfterReceivingTheMessage(true);
        }

        if (newMessage)
        {
            activeChatWindowsMap.value(id)->setWindowIcon(QIcon(":/images/message_icon.png"));
        }
        else
        {
            activeChatWindowsMap.value(id)->setWindowIcon(QIcon(":/images/jupiter_icon.png"));
        }

        qDebug() << "\tZmiana stanu wiadomości użytkownika" << id << "na stan" << newMessage;
        friendsMap.value(id)->setNewMessage(newMessage);
        qDebug() << "\tXXX" << friendsMap.value(id)->isNewMessage();
    }
    else
    {
        qDebug() << "\tOkno" << id << "nieaktywne";
        qDebug() << "\tZmiana stanu wiadomości użytkownika" << id << "na stan" << newMessage;
        friendsMap.value(id)->setNewMessage(newMessage);
        reloadFriendsListWidget();
    }
}

void MainWindow::onRemovedFriend(quint32 friendId)
{
    qDebug() << "SLOT onRemovedFriend(quint32 friendId)";
    qDebug() << "\tfriendId" << friendId;

    if (activeChatWindowsMap.contains(friendId))
    {
        activeChatWindowsMap.value(friendId)->close();
    }
    friendsMap = getFriendsMap();
    reloadFriendsListWidget();
}

void MainWindow::onAddedFriend(quint32 friendId)
{
    qDebug() << "SLOT onAddedFriend(quint32 friendId)";
    qDebug() << "\tfriendId" << friendId;

    friendsMap = getFriendsMap();
    reloadFriendsListWidget();
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
    qDebug() << "Jestem w metodzie changeMessageStatusInTheDatabaseToRead()";

    QString updateNewMessageState = QString("UPDATE %1_friends SET %1_friends.is_new_message = 0 WHERE %1_friends.id = '%2'")
                                        .arg(LoginPage::getUser().getId()).arg(friendId);

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



void MainWindow::on_friendsListWidget_itemDoubleClicked(QListWidgetItem *item)
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

void MainWindow::on_actionSearchUser_triggered()
{
    bool ok;
    QString searchedUser = QInputDialog::getText(this, "Search user", "Enter id or username:", QLineEdit::Normal, "", &ok);

    if (ok && !searchedUser.isEmpty())
    {
        QString searchUser = QString("SELECT users.id, users.username "
                                     "FROM users WHERE id = %1 OR username LIKE '%%2%'"
                                     "ORDER BY username ASC")
                                .arg(searchedUser.toUInt(&ok)).arg(searchedUser);
        QSqlDatabase database(LoginPage::getDatabase());
        QSqlQuery query(database);

        if (query.exec(searchUser))
        {
            if (query.size() == 0)
            {
                QMessageBox::information(this, "No results", "No users found.");
                return;
            }

            QDialog dialog(this);
            dialog.setWindowTitle("Search results");
            QVBoxLayout *layout = new QVBoxLayout;
            QListWidget *listWidget = new QListWidget(&dialog);

            while (query.next())
            {
                QString id = query.value("id").toString();
                QString username = query.value("username").toString();

                ExtendedQListWidgetItem *extendedItem = new ExtendedQListWidgetItem(username + "\tid: " + id, id.toUInt());
                listWidget->addItem(extendedItem);
            }

            layout->addWidget(listWidget);
            dialog.setLayout(layout);

            QMenu *contextMenu = new QMenu(this);
            QAction *actionInvite = new QAction("Invite", this);

            contextMenu->addAction(actionInvite);

            connect(actionInvite, &QAction::triggered, [=]() {
                QListWidgetItem *item = listWidget->currentItem();
                if (item) {
                    ExtendedQListWidgetItem *extendedItem = dynamic_cast<ExtendedQListWidgetItem*>(item);
                    if (extendedItem) {
                        quint32 userId = extendedItem->getId();
                        handleInviteAction(userId);
                    }
                }
            });


            listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(listWidget, &QTableWidget::customContextMenuRequested, [=](const QPoint &pos) {
                Q_UNUSED(pos);
                contextMenu->exec(QCursor::pos());
            });

            dialog.exec();
        }
    }
    else if (ok && searchedUser.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Field cannot be empty.");
    }
}

QList<Invitation *> MainWindow::getInvitationsList(const QString &invitationType)
{
    qDebug() << "Jestem w metodzie getInvitationsList(QString " +
                QString(invitationType) + ")";
    QString invitations = QString("SELECT %1_%2_invitations.invitation_id, "
                                             "%1_%2_invitations.id, "
                                             "%1_%2_invitations.username "
                                             "FROM %1_%2_invitations")
                                        .arg(LoginPage::getUser().getId())
                                        .arg(invitationType);

    QList<Invitation *> invitationsList;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(invitations))
    {
        if (query.numRowsAffected() > 0)
        {
            while (query.next())
            {
                invitationsList.append(new Invitation(query.value("invitation_id").toUInt(),
                                                      query.value("id").toUInt(),
                                                      query.value("username").toString()));
            }
        }
        else
        {
            qDebug() << "\tLista jest pusta";
        }
    }

    for (const auto& invitation : invitationsList)
    {
        qDebug() << "\t" << invitation->getId() << "\t"
                         << invitation->getUserId() << "\t"
                         << invitation->getUsername();
    }

    return invitationsList;
}




///////////////REFACTOR///////////
////// DRY //////////////////////
void MainWindow::onActionIInvitedClicked()
{    
    qDebug() << "Jestem w onActionIInvitedClicked()";

    iInvitedDialog = new QDialog(this);

    connect(iInvitedDialog, &QDialog::finished,
            this, &MainWindow::onIInvitedDialogClosed);

    iInvitedDialog->setWindowTitle("I invited");
    QVBoxLayout *layout = new QVBoxLayout;

    iInvitedListWidget = new QListWidget(iInvitedDialog);

    fillOutInvitationListWidget(iInvitedListWidget, sentInvitationsList);

    layout->addWidget(iInvitedListWidget);
    iInvitedDialog->setLayout(layout);
    iInvitedDialogOpen = true;

    iInvitedListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(iInvitedListWidget, &QListWidget::customContextMenuRequested,
                         this, &MainWindow::handleIInvitedListWidgetContextMenu);

    iInvitedDialog->exec();
}


void MainWindow::onActionInvitedMeClicked()
{
    qDebug() << "Jestem w onActionInvitedMeClicked()";

    invitedMeDialog = new QDialog(this);

    connect(invitedMeDialog, &QDialog::finished,
            this, &MainWindow::onInvitedMeDialogClosed);

    invitedMeDialog->setWindowTitle("Invited me");
    QVBoxLayout *layout = new QVBoxLayout;

    invitedMeListWidget = new QListWidget(invitedMeDialog);

    fillOutInvitationListWidget(invitedMeListWidget, receivedInvitationsList);

    layout->addWidget(invitedMeListWidget);
    invitedMeDialog->setLayout(layout);
    invitedMeDialogOpen = true;

    invitedMeListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(invitedMeListWidget, &QListWidget::customContextMenuRequested,
                         this, &MainWindow::handleInvitedMeListWidgetContextMenu);

    invitedMeDialog->exec();
}

bool MainWindow::inviteUserToFriends(quint32 userId)
{
    if (insertInviteIntoYourOwnTable(userId) && insertInviteIntoTheTableUser(userId))
    {
        return true;
    }
    return false;
}

bool MainWindow::insertInviteIntoTheTableUser(quint32 userId)
{
    qDebug() << "Jestem w insertInviteIntoTheTableUser(quint32 userId)";
    QString insert = QString("INSERT INTO %1_received_invitations (id, username) "
                             "VALUES ('%2', '%3')")
                        .arg(userId)
                        .arg(LoginPage::getUser().getId())
                        .arg(LoginPage::getUser().getUsername());
    qDebug() << "\t" << insert;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(insert))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tZaproszenie dotarło do użytkownika" << userId;
            return true;
        }
        else
        {
            qDebug() << "\tNieudana próba dostarczenia zaproszenia do użytkownika" << userId;
            return false;
        }
    }
    return false;
}

bool MainWindow::insertInviteIntoYourOwnTable(quint32 userId)
{
    qDebug() << "Jestem w insertInviteIntoYourOwnTable(quint32 userId)";
    QString insert = QString("INSERT INTO %1_sent_invitations (id, username) "
                             "SELECT '%2', users.username FROM users "
                             "WHERE users.id = '%2'")
                        .arg(LoginPage::getUser().getId())
                        .arg(userId);
    qDebug() << "\t" << insert;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(insert))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tWysłano zaproszenie do użytkownika" << userId;
            return true;
        }
        else
        {
            qDebug() << "\tNieudana próba zaproszenia użytkownika" << userId;
            return false;
        }
    }
    return false;
}

void MainWindow::changeSentInvitationList()
{
    qDebug() << "SLOT Lista sent sie zmieniła";
    sentInvitationsList = getInvitationsList("sent");
    if (iInvitedDialogOpen)
    {
        qDebug() << "\tRELOAD LIST";
        refreshInvitationListWidget(iInvitedListWidget, sentInvitationsList);
    }
}

void MainWindow::changeReceivedInvitationList()
{
    qDebug() << "SLOT Lista received sie zmienila";
    receivedInvitationsList = getInvitationsList("received");
    if (invitedMeDialogOpen)
    {
        qDebug() << "\tRELOAD LIST";
        refreshInvitationListWidget(invitedMeListWidget, receivedInvitationsList);
    }
}

void MainWindow::onIInvitedDialogClosed()
{
    iInvitedDialogOpen = false;
}

void MainWindow::onInvitedMeDialogClosed()
{
    invitedMeDialogOpen = false;
}



void MainWindow::fillOutInvitationListWidget(QListWidget *listWidget, QList<Invitation *> invitationsList)
{
    int size = invitationsList.size();

    for (int i = 0; i < size; ++i)
    {
        Invitation* invitation = invitationsList.at(i);
        quint32 userId = invitation->getUserId();
        ExtendedQListWidgetItem *extendedItem = new ExtendedQListWidgetItem(invitation->getUsername() +
                                                            "\tid: " + QString::number(invitation->getUserId()), userId);
       listWidget->addItem(extendedItem);
    }
}

void MainWindow::refreshInvitationListWidget(QListWidget *listWidget, QList<Invitation *> invitationsList)
{
    listWidget->clear();
    fillOutInvitationListWidget(listWidget, invitationsList);
}

void MainWindow::removeInvitationFromDatabase(quint32 userId, const QString &invitationType)
{
    qDebug() << "Jestem w metodzie removeInvitationInTheDatabase(quint32 userId)";
    QString deleteInvitation = QString("DELETE FROM %1_%2_invitations WHERE id = '%3'")
                                .arg(LoginPage::getUser().getId())
                                .arg(invitationType)
                                .arg(userId);
    qDebug() << "\t" << deleteInvitation;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(deleteInvitation))
    {
        if (query.numRowsAffected() == 1)
        {
            if (invitationType == "sent")
            {
                qDebug() << "\tUsunięto z tabeli wysłane zaproszenie do użytkownika"
                         << userId;
            }
            if (invitationType == "received")
            {
                qDebug() << "\tUsunięto z tabeli otrzymane zaproszenie o użytkownika"
                         << userId;
            }

        }
        else
        {
            if (invitationType == "sent")
            {
                qDebug() << "\tBłąd podczas usuwania zaproszenia do użytkownika"
                         << userId;
            }
            if (invitationType == "received")
            {
                qDebug() << "\tBłąd podczas usuwania zaproszenia od użytkownika"
                         << userId;
            }
        }
    }
    else
        qDebug() << "\tNieprawidłowe zapytanie do bazy danych";
}

void MainWindow::removeInvitationFromAnotherUsersTable(quint32 userId, const QString &invitationType)
{
    qDebug() << "Jestem w metodzie removeInvitationFromAnotherUsersTable(quint32 userId, const QString &invitationType)";
    QString deleteInvitation = QString("DELETE FROM %1_%2_invitations WHERE id = '%3'")
                                .arg(userId)
                                .arg(invitationType)
                                .arg(LoginPage::getUser().getId());
    qDebug() << "\t" << deleteInvitation;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(deleteInvitation))
    {
        if (query.numRowsAffected() == 1)
        {
            if (invitationType == "received")
            {
                qDebug() << "\tUsunięto z tabeli użytkownika" << userId << "otrzymane zaproszenie";
            }
            if (invitationType == "sent")
            {
                qDebug() << "\tUsunięto z tabeli użytkownika" << userId << "wysłane zaproszenie";
            }

        }
        else
        {
            if (invitationType == "received")
            {
                qDebug() << "\tBłąd podczas usuwania otrzymanego zaproszenia z tabeli użytkownika"
                         << userId;
            }
            if (invitationType == "sent")
            {
                qDebug() << "\tBłąd podczas usuwania wysłanego zaproszenia z tabeli użytkownika"
                         << userId;
            }
        }
    }
    else
        qDebug() << "\tNieprawidłowe zapytanie do bazy danych";

}

bool MainWindow::addFriendToDatabase(quint32 userId, quint32 friendId)
{
    qDebug() << "Jestem w metodzie addFriendToDatabase(quint32 userId, quint32 friendId)";

    QString addFriend = QString("INSERT INTO %1_friends (id, username, alias) "
                                "SELECT '%2', username, username "
                                "FROM users WHERE id = '%2'")
                            .arg(userId).arg(friendId);

    qDebug() << "\t" << addFriend;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(addFriend))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tDodano użytkownika" << friendId << "do grona przyjaciół.";
            return true;
        }
        else
        {
            qDebug() << "\tNieudana próba dodania użytkownika" << friendId
                     << "do grona przyjaciół.";
            return false;
        }
    }
    qDebug() << "\tBłąd podczas zapytania do bazy";
    return false;
}

void MainWindow::addChatTableToDatabase(quint32 userId, quint32 friendId)
{
    qDebug() << "Jestem w metodzie addChatTableToDatabase(quint32 userId, quint32 friendId)";

    QString createChatTable = QString("CREATE TABLE IF NOT EXISTS jupiter.%1_chat_%2 ("
                                      "message_id INT NOT NULL AUTO_INCREMENT, "
                                      "sender_id INT NULL DEFAULT NULL, "
                                      "timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, "
                                      "message TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL, "
                                      "`read` BOOLEAN NULL DEFAULT NULL, "
                                      "PRIMARY KEY (message_id))")
                                .arg(userId).arg(friendId);

    qDebug() << "\t" << createChatTable;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(createChatTable))
    {
        qDebug() << "\tTabela chatu została utworzona lub już istnieje.";
    }
    else
    {
        qDebug() << "\tDodawanie tabeli chatu zakończone niepowodzeniem" << query.lastError().text();
    }
}

void MainWindow::on_actionDelete_triggered()
{
    int result = QMessageBox::question(this, "Confirm Deletion",
                                       "Are you sure you want to delete account?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        deleteAccount();
        QCoreApplication::exit(0);
    }
}

void MainWindow::deleteAccount()
{
    deleteSentInvitationsTable();
    deleteReceivedInvitationsTable();
    deleteFriendsTable();
    deleteUserFromUsersTable();
    deleteUserFromFriendsTables();
    deleteAllChatTables();
}

void MainWindow::deleteSentInvitationsTable()
{
    qDebug() << "Jestem w metodzie deleteSentInvitationsTable()";
    QString dropTable = QString("DROP TABLE %1_sent_invitations")
                            .arg(LoginPage::getUser().getId());

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(dropTable))
    {
        qDebug() << "\tTabela sent_invitations została usunięta.";
    }
    else
    {
        qDebug() << "\tBłąd podczas usuwania tabeli sent_invitations:" << query.lastError().text();
    }
}

void MainWindow::deleteReceivedInvitationsTable()
{
    qDebug() << "Jestem w metodzie deleteReceivedInvitationsTable()";
    QString dropTable = QString("DROP TABLE %1_received_invitations")
                            .arg(LoginPage::getUser().getId());

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(dropTable))
    {
        qDebug() << "\tTabela received_invitations została usunięta.";
    }
    else
    {
        qDebug() << "\tBłąd podczas usuwania tabeli received_invitations:" << query.lastError().text();
    }
}

void MainWindow::deleteFriendsTable()
{
    qDebug() << "Jestem w metodzie deleteFriendsTable()";
    QString dropTable = QString("DROP TABLE %1_friends")
                            .arg(LoginPage::getUser().getId());

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(dropTable))
    {
        qDebug() << "\tTabela friends została usunięta.";
    }
    else
    {
        qDebug() << "\tBłąd podczas usuwania tabeli friends:" << query.lastError().text();
    }
}

void MainWindow::deleteUserFromUsersTable()
{
    qDebug() << "Jestem w metodzie deleteUserFromUsersTable()";
    QString deleteUser = QString("DELETE FROM users WHERE users.id = '%1'")
                            .arg(LoginPage::getUser().getId());

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(deleteUser))
    {
        qDebug() << "\tUżytkownik został usunięty z tabeli users.";
    }
    else
    {
        qDebug() << "\tBłąd podczas usuwania użytkownika:" << query.lastError().text();
    }
}

void MainWindow::deleteUserFromFriendsTables()
{
    qDebug() << "Jestem w metodzie deleteUserFromFriendsTables()";
    foreach (quint32 friendId, friendsMap.keys())
    {
        removeFriendFromDatabase(friendId, LoginPage::getUser().getId());
    }
}

void MainWindow::deleteChatTable(quint32 userId, quint32 friendId)
{
    qDebug() << "Jestem w metodzie deleteChatTable(quint32 userId, quint32 friendId)";
    QString dropTable = QString("DROP TABLE %1_chat_%2")
                            .arg(userId).arg(friendId);

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);

    if (query.exec(dropTable))
    {
        qDebug() << "\tTabela chatu została usunięta.";
    }
    else
    {
        qDebug() << "\tBłąd podczas usuwania tabeli chatu:" << query.lastError().text();
    }
}

void MainWindow::deleteAllChatTables()
{
    qDebug() << "Jestem w metodzie deleteAllChatTables()";
    foreach (quint32 friendId, friendsMap.keys())
    {
        deleteChatTable(LoginPage::getUser().getId(), friendId);
        deleteChatTable(friendId, LoginPage::getUser().getId());
    }
}

void MainWindow::on_actionLogout_triggered()
{
    this->close();
    LoginPage *loginPage = new LoginPage();
    loginPage->show();

    // trzeba pozamykac wszystkie połączenia z bazą
    // na razie opcja wyłączona
}

