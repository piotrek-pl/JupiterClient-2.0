#ifndef USER_H
#define USER_H

#include <QString>
#include <QtGlobal>

class User
{
public:
    User() { };
    User(quint32 id, QString username);
    quint32 getId() const { return id; }
    QString getUsername() const { return username; }
    void setId(quint32 id) { this->id = id; }
    void setUsername(QString username) { this->username = username; }
    quint32 operator()() const { return id; }

private:
    quint32 id;
    QString username;
};

#endif // USER_H
