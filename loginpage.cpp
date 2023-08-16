#include <QMessageBox>
#include "loginpage.h"
#include "ui_loginpage.h"
#include "mainwindow.h"
#include "signupwindow.h"

QSqlDatabase LoginPage::database = QSqlDatabase::addDatabase("QMYSQL", "mainThread");
User LoginPage::owner;

LoginPage::LoginPage(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginPage)
{
    ui->setupUi(this);
    signUpWindow = nullptr;

    if (connectToDatabase(database))
    {
        qDebug() << "Connected to database!";
    }
    else
    {
        qDebug() << "Failed to connect to database.";
        ::exit(1);
        // błąd połączenia z bazą danych (reconnect)
        // ui->loginButton->setEnabled(false);

    }

    connect(ui->userInput, SIGNAL(returnPressed()), ui->loginButton, SIGNAL(clicked()));
    connect(ui->passwordInput, SIGNAL(returnPressed()), ui->loginButton, SIGNAL(clicked()));

    setWindowIcon(QIcon(":/images/jupiter_icon.png"));
}

LoginPage::~LoginPage()
{
    delete ui;
    database.close();
}

bool LoginPage::connectToDatabase(QSqlDatabase database)
{
    database.setHostName("77.237.31.25");
    database.setPort(3306);
    database.setDatabaseName("jupiter");
    database.setUserName("pi");
    database.setPassword("raspberrypi");

    return database.open();
}

void LoginPage::on_loginButton_clicked()
{
    QString username = ui->userInput->text();
    QString password = ui->passwordInput->text();
    QString logOn = QString("SELECT id, username, password FROM users WHERE username = '%1' AND password = '%2'")
                            .arg(username, password);
    QSqlQuery query(database);
    if (query.exec(logOn))
    {
        if (query.size() > 0)
        {
            query.next();
            owner.setId(query.value("id").toUInt());
            owner.setUsername(query.value("username").toString());
            qDebug() << "Logged in as userId:" << owner.getId();
            this->close();
            mainWindow = new MainWindow;
            mainWindow->show();
        }
        else
        {
            QMessageBox::information(this, "Login failed.", "Login failed. Please try again...");
        }
    }
}

void LoginPage::on_signUpButton_clicked()
{
    signUpWindow = new SignUpWindow;
    connect(signUpWindow, &SignUpWindow::closed, this, &LoginPage::onSignUpWindowClosed);
    signUpWindow->show();
    this->setDisabled(true);
}

void LoginPage::onSignUpWindowClosed()
{
    this->setDisabled(false);
    signUpWindow = nullptr;
}

void LoginPage::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    if (signUpWindow)
    {
        signUpWindow->close();
    }
}
