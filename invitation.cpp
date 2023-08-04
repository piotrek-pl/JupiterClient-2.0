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

Invitation& Invitation::operator=(const Invitation &other)
{
    if (this != &other)
    {
        id = other.id;
        userId = other.userId;
        username = other.username;
    }
    return *this;
}

bool Invitation::operator==(const Invitation &other) const
{
    return (id == other.id && userId == other.userId && username == other.username);
}

bool Invitation::operator!=(const Invitation &other) const
{
    return !(*this == other);
}
