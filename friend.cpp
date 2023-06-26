#include "friend.h"

Friend::Friend(quint32 id, QString username, QString alias, bool newMessage, bool available)
    : User(id, username)
    , alias(alias)
    , newMessage(newMessage)
    , available(available)
{
    setState();
}

void Friend::setNewMessage(bool newMessage)
{
    this->newMessage = newMessage;
    setState();
}
void Friend::setAvailable(bool available)
{
    this->available = available;
    setState();
}

void Friend::setState()
{
    if (available)
    {
        if (newMessage)
            state = State::AvailableWithMessage;
        else
            state = State::AvailableNoMessage;
    } else
        if (newMessage)
            state = State::UnavailableWithMessage;
        else
            state = State::UnavailableNoMessage;
}

Friend &Friend::operator=(quint32 id)
{
    setId(id);
    return *this;
}
