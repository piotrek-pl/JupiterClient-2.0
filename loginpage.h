#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QMainWindow>
#include <QtSql>
#include "mainwindow.h"
#include "signupwindow.h"
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
    void onSignUpWindowClosed();

private:
    Ui::LoginPage *ui;
    SignUpWindow *signUpWindow;
    static QSqlDatabase database;
    static User owner;
    void closeEvent(QCloseEvent *event);
    MainWindow *mainWindow;
};
#endif // LOGINPAGE_H
