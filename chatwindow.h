#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "messagescontroller.h"

#include <QMainWindow>
#include <QScrollBar>
#include <QTcpSocket>
#include <QTextCharFormat>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    explicit ChatWindow(quint32 converserId, QWidget *parent = nullptr);
    void sendMessage(quint32 senderId, quint32 receiverId, const QString& message);
    ~ChatWindow();

signals:
    void finish();

private slots:
    void on_sendButton_clicked();
    void onTextChanged();
    void readNewMessages(quint32 lastMessageId);

private:
    Ui::ChatWindow *ui;
    QTcpSocket *socket;
    MessagesController *messagesController;
    quint32 converserId;
    quint32 lastReadMessageId;

    QTextCharFormat incomingMessages;
    QTextCharFormat outgoingMessages;

    QScrollBar *scrollBar;


    void setTheMessagesFormat();

    //QString getUsernameAliasFromDatabase(quint32 userId);
    void getAllMessagesFromDatabaseAndDisplay();
    void makeThread();
    void closeEvent(QCloseEvent *event);
    //void mousePressEvent(QMouseEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    bool eventFilter(QObject *obj, QEvent *event);

    DatabaseConnectionManager databaseConnectionManager;
};

#endif // CHATWINDOW_H
