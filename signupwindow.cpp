#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "signupwindow.h"
#include "ui_signupwindow.h"
#include "loginpage.h"

SignUpWindow::SignUpWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SignUpWindow)
{
    ui->setupUi(this);
}

SignUpWindow::~SignUpWindow()
{
    delete ui;
}

void SignUpWindow::SignUpWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    emit closed();
}

void SignUpWindow::on_signUpButton_clicked()
{
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();
    QString confirmation = ui->confirmInput->text();

    if (username.isEmpty())
    {
        qDebug() << "Enter name";
        QMessageBox::warning(this, "Warning", "Username cannot be empty.");
        ui->passwordInput->clear();
        ui->confirmInput->clear();
    }
    else if (password.isEmpty())
    {
        qDebug() << "Enter password";
        QMessageBox::warning(this, "Warning", "Password cannot be empty.");
        ui->confirmInput->clear();
    }
    else if (confirmation.isEmpty())
    {
        qDebug() << "Confirm password";
        ui->passwordInput->clear();
        QMessageBox::warning(this, "Warning", "Password confirmation cannot be empty");
    }
    else if (password != confirmation)
    {
        qDebug() <<  "Password and Confirm Password do not match.";
        QMessageBox::warning(this, "Warning", "Password and Confirm Password do not match.");
        ui->usernameInput->clear();
        ui->passwordInput->clear();
        ui->confirmInput->clear();
    }
    else if (userExists())
    {
        qDebug() <<  "User already exists";
        QMessageBox::warning(this, "Warning", "User already exists.");
        ui->usernameInput->clear();
        ui->passwordInput->clear();
        ui->confirmInput->clear();
    }
    else if (createUserAccount())
    {
        qDebug() << "Account has been created";
        QMessageBox::information(this, "Information", "Account has been created.");
        this->close();
    }
    else
    {
        qDebug() << "Account creation error";
        QMessageBox::warning(this, "Warning", "Account creation error.");
        this->close();
    }
}

bool SignUpWindow::createUserAccount()
{
    qDebug() << "Jestem w metodzie createUserAccount()";
    QString addUser = QString("INSERT INTO `users` ("
                                "`id`, "
                                "`timestamp`, "
                                "`username`, "
                                "`password`, "
                                "`available`, "
                                "`ip`, "
                                "`port`) "
                              "VALUES ("
                                "NULL, "
                                "current_timestamp(), "
                                "'%1', "
                                "'%2', "
                                "'0', '0', '0')")
            .arg(ui->usernameInput->text(), ui->passwordInput->text());

    qDebug() << "\t" << addUser;
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(addUser))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tDodano użytkownika do bazy";
            return true;
        }
        else
        {
            qDebug() << "\tDodawanie użytkownika do bazy zakończone niepowodzeniem";
        }
    }

    qDebug() << "\t" << query.lastError().text();
    return false;
}

bool SignUpWindow::userExists()
{
    qDebug() << "Jestem w metodzie userExists()";
    QString wheterTheUsernameExists = QString("SELECT username FROM users "
                                              "WHERE username = '%1'")
                                      .arg(ui->usernameInput->text());
    qDebug() << "\t" << wheterTheUsernameExists;
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (!query.exec(wheterTheUsernameExists))
    {
        qDebug() << "\tBłąd w wykonaniu zapytania: " << query.lastError().text();
        return false;
    }

    return query.next();
}
