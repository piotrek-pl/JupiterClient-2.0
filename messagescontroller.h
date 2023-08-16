#ifndef MESSAGESCONTROLLER_H
#define MESSAGESCONTROLLER_H

#include "databaseconnectionmanager.h"
#include <QObject>
#include <QSqlDatabase>
#include <QDebug>

class MessagesController : public QObject
{
    Q_OBJECT
public:
    explicit MessagesController(quint32 converserId, quint32 lastReadMessageId, QObject *parent = nullptr);
    void checkNewMessages();
    ~MessagesController();

    bool isChatWindowClosed;

signals:
    void wroteNewMessageInDatabase(quint32 lastMessageId);

public slots:
    void run();

private:
    quint32 converserId;
    quint32 lastReadMessageId;
    QSqlDatabase database;

    DatabaseConnectionManager databaseConnectionManager;

};

#endif // MESSAGESCONTROLLER_H
