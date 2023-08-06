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
                     this, &MainWindow::handleListWidgetContextMenu);
    QObject::connect(ui->actionIInvited, &QAction::triggered,
                     this, &MainWindow::onActionIInvitedClicked);
    QObject::connect(ui->actionInvitedMe, &QAction::triggered,
                     this, &MainWindow::onActionInvitedMeClicked);   

    setWindowIcon(QIcon(":/images/jupiter_icon.png"));
    setWindowFlags(windowFlags() &(~Qt::WindowMaximizeButtonHint));
}

void MainWindow::handleListWidgetContextMenu(const QPoint &pos)
{
    qDebug() << "Jestem w metodzie handleListWidgetContextMenu(const QPoint &pos)";

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
            }
        }
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

QList<Invitation *> MainWindow::getInvitationsList(QString invitationType)
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

    int size = sentInvitationsList.size();

    //QDialog dialog(this);
    iInvitedDialog = new QDialog(this);

    connect(iInvitedDialog, &QDialog::finished,
            this, &MainWindow::onIInvitedDialogClosed);

    //dialog.setWindowTitle("I invited");
    iInvitedDialog->setWindowTitle("I invited");
    QVBoxLayout *layout = new QVBoxLayout;
    //QListWidget *listWidget = new QListWidget(&dialog);
    QListWidget *listWidget = new QListWidget(iInvitedDialog);

    for (int i = 0; i < size; ++i)
    {
        Invitation* invitation = sentInvitationsList.at(i);
        quint32 userId = invitation->getUserId();
        ExtendedQListWidgetItem *extendedItem = new ExtendedQListWidgetItem(invitation->getUsername() +
                                                        "\tid: " + QString::number(invitation->getUserId()), userId);
        listWidget->addItem(extendedItem);
    }

    layout->addWidget(listWidget);
    //dialog.setLayout(layout);
    iInvitedDialog->setLayout(layout);

    //dialog.exec();
    iInvitedDialogOpen = true;
    iInvitedDialog->exec();




}


void MainWindow::onActionInvitedMeClicked()
{
    qDebug() << "Jestem w onActionInvitedMeClicked()";

    int size = receivedInvitationsList.size();

    //QDialog dialog(this);
    invitedMeDialog = new QDialog(this);

    connect(invitedMeDialog, &QDialog::finished,
            this, &MainWindow::onInvitedMeDialogClosed);

    //dialog.setWindowTitle("Invited me");
    invitedMeDialog->setWindowTitle("Invited me");
    QVBoxLayout *layout = new QVBoxLayout;
    //QListWidget *listWidget = new QListWidget(&dialog);
    QListWidget *listWidget = new QListWidget(invitedMeDialog);

    for (int i = 0; i < size; ++i)
    {
        Invitation* invitation = receivedInvitationsList.at(i);
        quint32 userId = invitation->getUserId();
        ExtendedQListWidgetItem *extendedItem = new ExtendedQListWidgetItem(invitation->getUsername() +
                                                        "\tid: " + QString::number(invitation->getUserId()), userId);
        listWidget->addItem(extendedItem);
    }

    layout->addWidget(listWidget);
    //dialog.setLayout(layout);
    invitedMeDialog->setLayout(layout);

    //dialog.exec();
    invitedMeDialogOpen = true;
    invitedMeDialog->exec();


}

bool MainWindow::inviteUserToFriends(quint32 userId)
{
    if (insertInviteIntoYourOwnTable(userId) && insertInviteIntoTheTableUser(userId))
    {
        return true;
    }
    return false;

    // dodaj do swojej tabeli - ZROBIONE
    // dodaj do tabeli usera - ZROBIONE
    // dodaj do receivedInvitationsList.append(...)
    // -- tym ostatnim powinen zajac sie slot uruchamiany innym watkiem
    // -- tutaj niepotrzebny jest reload, gdyz to jest lista userów
    // -- reload bedzie potrzebny przy przegladaniu listy i przy
    // cancel bądź accept zaproszenia
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
    // Jesli lista jest otwarta to reload
    if (iInvitedDialogOpen)
    {
        qDebug() << "\tRELOAD LIST";
        // reload
    }
}

void MainWindow::changeReceivedInvitationList()
{
    qDebug() << "SLOT Lista received sie zmienila";
    receivedInvitationsList = getInvitationsList("received");
    // Jesli lista jest otwarta to reload
    if (invitedMeDialogOpen)
    {
        qDebug() << "\tRELOAD LIST";
        // reload
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
