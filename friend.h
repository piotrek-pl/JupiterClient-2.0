#ifndef FRIEND_H
#define FRIEND_H

#include <user.h>

class Friend : public User
{
public:
    enum class State
    {
        AvailableNoMessage,
        AvailableWithMessage,
        UnavailableNoMessage,
        UnavailableWithMessage
    };

    Friend(quint32 id, QString username, QString alias, bool newMessage = false, bool available = false);
    QString getAlias() const { return alias; }
    bool isNewMessage() const { return newMessage; }
    bool isAvailable() const { return available; }
    void setNewMessage(bool newMessage);
    void setAvailable(bool available);
    void setState();    
    enum State getState() const { return state; }
    Friend& operator=(quint32 id);

private:
    QString alias;
    bool newMessage;
    bool available;
    State state;
};

#endif // FRIEND_H
