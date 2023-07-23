#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QMainWindow>
#include <QtSql>
#include "user.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginPage; }
QT_END_NAMESPACE

class LoginPage : public QMainWindow
{
    Q_OBJECT

public:
    LoginPage(QWidget *parent = nullptr);
    ~LoginPage();
    static QSqlDatabase getDatabase() { return database; }
    static bool connectToDatabase(QSqlDatabase database);
    static User& getUser() { return owner; }


private slots:
    void on_loginButton_clicked();
    void on_signUpButton_clicked();

private:
    Ui::LoginPage *ui;
    static QSqlDatabase database;
    static User owner;
};
#endif // LOGINPAGE_H
