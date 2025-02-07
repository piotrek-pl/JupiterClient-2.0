#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "friendsstatuses.h"
#include "friend.h"
#include "chatwindow.h"
#include "invitation.h"
#include "invitationcontroller.h"
#include "connectionlostdialog.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTcpSocket>
#include <QMoveEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //static QList<quint32> activeWindowsList;
    static QMap<quint32, ChatWindow *> activeChatWindowsMap;    

    static QList<Invitation *> sentInvitationsList;
    static QList<Invitation *> receivedInvitationsList;
    static QList<Invitation *> getInvitationsList(const QString &invitationType);

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QTcpSocket* getSocket() { return socket; }
    //static QList<Friend *> friends;
    static QMap<quint32, Friend *> friendsMap;
    QMap<quint32, Friend *> getFriendsMap();
    //QList<Friend *> getFriendsList();
    static void changeMessageStatusInTheDatabaseToRead(quint32 friendId);
    static bool isNewMessage(quint32 friendId);


public slots:
    //void updateFriendsList(quint32, bool);
    void changeAvailabilityStatus(quint32, bool);
    void changeFriendMessageStatus(quint32, bool);
    void onRemovedFriend(quint32 friendId);
    void onAddedFriend(quint32 friendId);

    void changeSentInvitationList();
    void changeReceivedInvitationList();


private slots:
    void socketConnected();
    void socketDisconnected();
    //void socketReadReady();
    void on_friendsListWidget_itemDoubleClicked(QListWidgetItem *item);
    void handleFriendsListWidgetContextMenu(const QPoint &pos);
    void handleInviteAction(quint32 userId);

    void on_actionSearchUser_triggered();
    void onActionIInvitedClicked();
    void onActionInvitedMeClicked();

    void onIInvitedDialogClosed();
    void onInvitedMeDialogClosed();

    void handleIInvitedListWidgetContextMenu(const QPoint &pos);
    void handleInvitedMeListWidgetContextMenu(const QPoint &pos);


    void on_actionDelete_triggered();

    void on_actionLogout_triggered();

    void onTimeout();

    void handleDatabaseConnectionLost();
    void handleDatabaseConnectionRestored();


private:
    static QTcpSocket *socket;
    Ui::MainWindow *ui;
    FriendsStatuses *friendsStatuses;
    InvitationController *invitationController;

    QDialog *iInvitedDialog;
    QDialog *invitedMeDialog;
    bool iInvitedDialogOpen = false;
    bool invitedMeDialogOpen = false;
    QListWidget *iInvitedListWidget;
    QListWidget *invitedMeListWidget;



    QTimer *timer;
    bool connected = false;
    ConnectionLostDialog *errorConnectionDialog;

    //DatabaseConnectionManager *databaseConnectionManager;

    void connectToServer();
    bool reconnectToServer();
    void sendFirstMessage(quint32 senderId);
    void addFriendToList(QListWidgetItem *item, QString friendUsername, QIcon icon);
    bool changeUsernameAliasInTheDatabase(QString newAlias, quint32 friendId);
    bool removeFriendFromDatabase(quint32 userId, quint32 friendId);
    bool addFriendToDatabase(quint32 userId, quint32 friendId);
    void addChatTableToDatabase(quint32 userId, quint32 friendId);

    //void changeNewMessageState(quint32 userId, quint32 state);
    //void changeNewMessageState(quint32 friendId, bool state);
    //void changeMessageStatusToRead(quint32 friendId);
    void createFriendsList();

    void fillOutFriendsListWidget();
    void reloadFriendsListWidget();

    //QStringList getFieldNames(const QSqlRecord &record);
    bool inviteUserToFriends(quint32 userId);
    bool insertInviteIntoYourOwnTable(quint32 userId);
    bool insertInviteIntoTheTableUser(quint32 userId);

    void fillOutInvitationListWidget(QListWidget *listWidget, QList<Invitation *> invitationsList);
    void refreshInvitationListWidget(QListWidget *listWidget, QList<Invitation *> invitationsList);

    //void removeSentInvitationFromDatabase(quint32 userId);
    void removeInvitationFromDatabase(quint32 userId, const QString &invitationType);
    void removeInvitationFromAnotherUsersTable(quint32 userId, const QString &invitationType);

    void deleteAccount();

    void deleteSentInvitationsTable();
    void deleteReceivedInvitationsTable();
    void deleteFriendsTable();
    void deleteUserFromUsersTable();
    void deleteUserFromFriendsTables();
    void deleteChatTable(quint32 userId, quint32 friendId);
    void deleteAllChatTables();

    void makeThreads();

    //enum ID { NO_ID = 0 };
    enum MESSAGE_STATE { AVAILABLE = 1,
                         UNAVAILABLE = 0 };
protected:
    void moveEvent(QMoveEvent* event) override;


};

#endif // MAINWINDOW_H
