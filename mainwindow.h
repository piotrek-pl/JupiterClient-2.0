#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "friendsstatuses.h"
#include "friend.h"
#include "chatwindow.h"
#include "invitation.h"
#include "invitationcontroller.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTcpSocket>

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

    void changeSentInvitationList();
    void changeReceivedInvitationList();

private slots:
    void socketConnected();
    void socketDisconnected();
    //void socketReadReady();
    void on_friendsListWidget_itemDoubleClicked(QListWidgetItem *item);
    void handleListWidgetContextMenu(const QPoint &pos);
    void handleInviteAction(quint32 userId);

    void on_actionSearchUser_triggered();
    void onActionIInvitedClicked();
    void onActionInvitedMeClicked();

    void onIInvitedDialogClosed();
    void onInvitedMeDialogClosed();


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




    void connectToServer();
    void sendFirstMessage(quint32 senderId);
    void addFriendToList(QListWidgetItem *item, QString friendUsername, QIcon icon);
    bool changeUsernameAliasInTheDatabase(QString newAlias, quint32 friendId);
    bool removeFriendFromDatabase(quint32 userId, quint32 friendId);

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



    void makeThreads();

    //enum ID { NO_ID = 0 };
    enum MESSAGE_STATE { AVAILABLE = 1,
                         UNAVAILABLE = 0 };   

};

#endif // MAINWINDOW_H
