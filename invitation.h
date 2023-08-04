#ifndef INVITATION_H
#define INVITATION_H

#include <QString>
#include <qglobal.h>


class Invitation
{
public:
    Invitation();
    Invitation(quint32 id, quint32 userId, QString username);
    quint32 getId() const;
    quint32 getUserId() const;
    const QString &getUsername() const;

    Invitation& operator=(const Invitation &other);
    bool operator==(const Invitation &other) const;
    bool operator!=(const Invitation &other) const;

private:
    quint32 id;
    quint32 userId;
    QString username;
};

#endif // INVITATION_H
