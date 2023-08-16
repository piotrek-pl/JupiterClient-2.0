#ifndef INVITATIONCONTROLLER_H
#define INVITATIONCONTROLLER_H

#include <QObject>
#include <QSqlDatabase>
#include "databaseconnectionmanager.h"
#include "invitation.h"

class InvitationController : public QObject
{
    Q_OBJECT
public:
    explicit InvitationController(QObject *parent = nullptr);

signals:
    void receivedInvitationsChanged();
    void sentInvitationsChanged();

public slots:
    void run();

private:
    QSqlDatabase database;
    void checkInvitations();
    QList<Invitation *> getInvitationsList(QString invitationType);
    DatabaseConnectionManager databaseConnectionManager;

};

#endif // INVITATIONCONTROLLER_H
