#include "invitation.h"

Invitation::Invitation()
{

}

Invitation::Invitation(quint32 id, quint32 userId, QString username)
    : id(id)
    , userId(userId)
    , username(username)
{

}

quint32 Invitation::getId() const
{
    return id;
}

quint32 Invitation::getUserId() const
{
    return userId;
}

const QString &Invitation::getUsername() const
{
    return username;
}
