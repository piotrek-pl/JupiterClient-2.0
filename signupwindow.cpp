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
    setWindowIcon(QIcon(":/images/jupiter_icon.png"));
    databaseConnectionManager = new DatabaseConnectionManager();
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
    if (!databaseConnectionManager->checkConnection(LoginPage::getDatabase()))
    {
        if (!LoginPage::connectToDatabase(LoginPage::getDatabase()))
        {
            QMessageBox::critical(nullptr, "Connection Error", "No connection to the database.");
            return;
        }
    }
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
        quint32 newUserId = getUserIdBasedOnUsername(ui->usernameInput->text());
        createTablesForTheUser(newUserId);
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

quint32 SignUpWindow::getUserIdBasedOnUsername(const QString &username)
{
    qDebug() << "Jestem w metodzie getUserIdBasedOnUsername(const QString &username)";
    QString getUserId = QString("SELECT id FROM users WHERE username = '%1'")
                            .arg(username);

    qDebug() << "\t" << getUserId;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(getUserId))
    {
        if (query.numRowsAffected() == 1)
        {
            query.next();
            qDebug() << "\tPobrano id użytkownika" << username;
            return query.value(0).toUInt();

        }
        else
        {
            qDebug() << "\tPobieranie id użytkownika zakończone niepowodzeniem";
            return 0;
        }
    }

    qDebug() << "\t" << query.lastError().text();
    return 0;
}

void SignUpWindow::createTablesForTheUser(quint32 userId)
{
    createFriendsTable(userId);
    createReceivedInvitationsTable(userId);
    createSentInvitationsTable(userId);
}

void SignUpWindow::createFriendsTable(quint32 userId)
{
    qDebug() << "Jestem w metodzie createTablesForTheUser(quint32 userId)";
    QString createFriendsTable = QString("CREATE TABLE IF NOT EXISTS jupiter.%1_friends ("
                                         "id INT NOT NULL AUTO_INCREMENT, "
                                         "username VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL, "
                                         "alias VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL, "
                                         "timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, "
                                         "is_new_message BOOLEAN NOT NULL DEFAULT FALSE, "
                                         "PRIMARY KEY (id))")
                                    .arg(userId);

   qDebug() << "\t" <<createFriendsTable;

   QSqlDatabase database(LoginPage::getDatabase());
   QSqlQuery query(database);
   if (query.exec(createFriendsTable))
   {
       qDebug() << "\tTabela friends została utworzona lub już istnieje.";
   }
   else
   {
       qDebug() << "\tDodawanie tabeli friends zakończone niepowodzeniem" << query.lastError().text();
   }
}

void SignUpWindow::createReceivedInvitationsTable(quint32 userId)
{
    qDebug() << "Jestem w metodzie createReceivedInvitationsTable(quint32 userId)";
    QString createReceivedInvitationsTable = QString("CREATE TABLE IF NOT EXISTS jupiter.%1_received_invitations ("
                                                     "invitation_id INT NOT NULL AUTO_INCREMENT, "
                                                     "id INT NOT NULL, "
                                                     "username VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL, "
                                                     "PRIMARY KEY  (invitation_id))")
                                                .arg(userId);
    qDebug() << "\t" << createReceivedInvitationsTable;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(createReceivedInvitationsTable))
    {
        qDebug() << "\tTabela received_invitations została utworzona lub już istnieje.";
    }
    else
    {
        qDebug() << "\tDodawanie tabeli received_invitations zakończone niepowodzeniem" << query.lastError().text();
    }
}


void SignUpWindow::createSentInvitationsTable(quint32 userId)
{
    qDebug() << "Jestem w metodzie createSentInvitationsTable(quint32 userId)";
    QString createSentInvitationsTable = QString("CREATE TABLE IF NOT EXISTS jupiter.%1_sent_invitations ("
                                                     "invitation_id INT NOT NULL AUTO_INCREMENT, "
                                                     "id INT NOT NULL, "
                                                     "username VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL, "
                                                     "PRIMARY KEY  (invitation_id))")
                                                .arg(userId);
    qDebug() << "\t" << createSentInvitationsTable;

    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    if (query.exec(createSentInvitationsTable))
    {
        qDebug() << "\tTabela sent_invitations została utworzona lub już istnieje.";
    }
    else
    {
        qDebug() << "\tDodawanie tabeli sent_invitations zakończone niepowodzeniem" << query.lastError().text();
    }
}


