#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "friendsstatuses.h"
#include "friend.h"

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
    static QList<quint32> activeWindowsList;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QTcpSocket* getSocket() { return socket; }
    static QList<Friend *> friends;
    //static QMap<quint32, Friend *> friendsMap;
    QList<Friend *> getFriendsList();
    //QMap<quint32, Friend *> getFriendsMap();

public slots:
    //void updateFriendsList();
    void updateFriendsList(quint32, bool);
    void changeAvailabilityStatus(quint32, bool);
    void changeMessageStatus(quint32, bool);

private slots:
    void socketConnected();
    void socketDisconnected();
    //void socketReadReady();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    quint32 getFriendIdFromItem(QListWidgetItem *item);

private:
    static QTcpSocket *socket;
    Ui::MainWindow *ui;
    FriendsStatuses *friendsStatuses;
    void connectToServer();
    void sendFirstMessage(quint32 senderId);
    void addFriendToList(QListWidgetItem *item, QString friendUsername, QIcon icon);
    bool checkForNewMessage(quint32 userId);
    void changeNewMessageState(quint32 userId, quint32 state);
    void createFriendsList();

    void fillOutFriendsListWidget();
    void reloadFriendsListWidget();

    void makeThread();

    //enum ID { NO_ID = 0 };
    enum MESSAGE_STATE { AVAILABLE = 1,
                         UNAVAILABLE = 0 };


};

#endif // MAINWINDOW_H
