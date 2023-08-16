#ifndef DATABASECONNECTIONMANAGER_H
#define DATABASECONNECTIONMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class DatabaseConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseConnectionManager(QObject *parent = nullptr);
    bool reconnectDatabase(QSqlDatabase database);
    bool checkConnection(QSqlDatabase database);

signals:
    void databaseConnectionLost();
    void databaseConnectionRestored();
};

#endif // DATABASECONNECTIONMANAGER_H
