#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "friendsstatuses.h"
#include "friend.h"
#include "chatwindow.h"

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

private slots:
    void socketConnected();
    void socketDisconnected();
    //void socketReadReady();
    void on_friendsListWidget_itemDoubleClicked(QListWidgetItem *item);
    void handleListWidgetContextMenu(const QPoint &pos);

private:
    static QTcpSocket *socket;
    Ui::MainWindow *ui;
    FriendsStatuses *friendsStatuses;
    void connectToServer();
    void sendFirstMessage(quint32 senderId);
    void addFriendToList(QListWidgetItem *item, QString friendUsername, QIcon icon);
    bool changeUsernameAliasInTheDatabase(QString newAlias, quint32 friendId);
    bool removeFriendFromDatabase(quint32 friendId);

    //void changeNewMessageState(quint32 userId, quint32 state);
    //void changeNewMessageState(quint32 friendId, bool state);
    //void changeMessageStatusToRead(quint32 friendId);
    void createFriendsList();

    void fillOutFriendsListWidget();
    void reloadFriendsListWidget();

    void makeThread();    

    //enum ID { NO_ID = 0 };
    enum MESSAGE_STATE { AVAILABLE = 1,
                         UNAVAILABLE = 0 };    
};

#endif // MAINWINDOW_H
