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
        AvailableWithMessageOpenChatWindow,
        UnavailableNoMessage,
        UnavailableWithMessage,
        UnavailableWithMessageOpenChatWindow
    };

    Friend(quint32 id, QString username, QString alias, bool newMessage = false, bool available = false,
           bool openChatWindow = false, bool chatWindowOpenedAfterReceivingTheMessage = false, bool chatWindowJustClosed = false);
    QString getAlias() const { return alias; }
    bool isNewMessage() const { return newMessage; }
    bool isAvailable() const { return available; }
    bool isOpenChatWindow() const { return openChatWindow; }
    void setNewMessage(bool newMessage);
    void setAvailable(bool available);
    void setOpenChatWindow(bool openChatWindow);
    void setState();    
    enum State getState() const { return state; }
    Friend& operator=(quint32 id);

    bool wheterChatWindowOpenedAfterReceivingTheMessage() { return chatWindowOpenedAfterReceivingTheMessage; }
    void setChatWindowOpenedAfterReceivingTheMessage(bool flag) { this->chatWindowOpenedAfterReceivingTheMessage = flag; }

    bool wheterChatWindowJustClosed() { return chatWindowJustClosed; };
    void setChatWindowJustClosed(bool flag) { this->chatWindowJustClosed = flag; };




private:
    QString alias;
    bool newMessage;
    bool available;
    bool openChatWindow;
    State state;

    bool chatWindowOpenedAfterReceivingTheMessage;
    bool chatWindowJustClosed;
};

#endif // FRIEND_H
