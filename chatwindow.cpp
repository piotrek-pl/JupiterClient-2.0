#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "mainwindow.h"
#include "loginpage.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    socket = MainWindow::getSocket();
}

ChatWindow::ChatWindow(quint32 converserId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    ui->sendButton->setDisabled(true);
    connect(ui->messageInput, &QTextEdit::textChanged, this, &ChatWindow::onTextChanged);
    this->setWindowTitle(getUsernameAliasFromDatabase(converserId));
    this->converserId = converserId;
    getAllMessagesFromDatabaseAndDisplay();
    socket = MainWindow::getSocket();

    makeThread();
}

ChatWindow::~ChatWindow()
{
    delete ui;
    MainWindow::activeWindowsList.removeOne(converserId);

    if (MainWindow::isNewMessage(converserId))
    {
        MainWindow::changeMessageStatusInTheDatabaseToRead(converserId);
    }

    for (Friend* friendPtr : MainWindow::friends)
    {
        if ((*friendPtr)() == converserId)
        {
            friendPtr->setOpenChatWindow(false);
            qDebug() << "setOpenChatWindow ustawiono na false";
        }
    }

    //changeNewMessageStateToRead();
    /*if (MainWindow::isNewMessage(converserId))
    {
        MainWindow::changeMessageStatusToRead(converserId);
    }*/
}

/*void ChatWindow::changeNewMessageStateToRead()
{
    QString idToUsernameQuery = "SELECT username FROM users WHERE id = " + QString::number(converserId);
    QSqlDatabase database = LoginPage::getDatabase();
    QSqlQuery query(database);

    if (query.exec(idToUsernameQuery))
    {
        if (query.size() > 0)
        {
            query.next();
            QString username = LoginPage::getUser().getUsername();
            QString updateNewMessageStateSqlCommand = "UPDATE " + username + "_friends SET " +
                    username + "_friends.is_new_message = 0 " +  + "WHERE " +
                    username + "_friends.id = " + "'" + QString::number(converserId) + "'";

            if (query.exec(updateNewMessageStateSqlCommand))
            {
                if (query.numRowsAffected() == 1)
                {
                    qDebug() << "Updated is_new_message state";
                }
                else
                {
                    qDebug() << "Update is_new_message state for user failed.";
                }
            }
        }
    }
}*/

void ChatWindow::makeThread()
{
    messagesController = new MessagesController(converserId, lastReadMessageId);
    QThread *thread = new QThread;
    messagesController->moveToThread(thread);
    connect(thread, &QThread::started, messagesController, &MessagesController::run);
    connect(messagesController, &MessagesController::wroteNewMessageInDatabase, this, &ChatWindow::readNewMessages);
    thread->start();
}

void ChatWindow::readNewMessages(quint32 lastMessageId)
{
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    QString unreadMessagesQuery = "SELECT * FROM " + QString::number(LoginPage::getUser().getId()) + "_chat_" + QString::number(converserId)
            + " WHERE message_id >= " + QString::number(lastReadMessageId) + " AND message_id <= "
            + QString::number(lastMessageId);

    qDebug() << unreadMessagesQuery;

    if (query.exec(unreadMessagesQuery))
    {
        while (query.next())
        {
            if (query.value("sender_id").toString() == QString::number(LoginPage::getUser().getId()))
            {
                ui->chatDisplay->append("<b>Ja</b> (" + query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                ui->chatDisplay->append("<font color=Green>" + query.value("message").toString() + "</font>");
            }
            else
            {
                ui->chatDisplay->append("<b>" + this->windowTitle() + "</b> (" +
                                        query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                ui->chatDisplay->append("<font color=Red>" + query.value("message").toString() + "</font>");
            }

        }
    }
    lastReadMessageId = lastMessageId;
}

void ChatWindow::onTextChanged()
{
    ui->sendButton->setDisabled(ui->messageInput->toPlainText().isEmpty());
}

void ChatWindow::on_sendButton_clicked()
{

    QDataStream stream(socket);
    sendMessage(LoginPage::getUser().getId(), converserId, ui->messageInput->toPlainText());
    ui->messageInput->clear();
}

QString ChatWindow::getUsernameAliasFromDatabase(quint32 userId)
{
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    QString usernameAliasQuery = "SELECT " + LoginPage::getUser().getUsername() + "_friends.username_alias FROM " +
            LoginPage::getUser().getUsername() + "_friends " + "WHERE id = " + "'" + QString::number(userId) + "'";

    if (query.exec(usernameAliasQuery))
    {
        if (query.size() > 0)
        {
            query.next();
            return query.value("username_alias").toString();
        }
    }

    return 0;
}

void ChatWindow::getAllMessagesFromDatabaseAndDisplay()
{
    QSqlDatabase database(LoginPage::getDatabase());
    QSqlQuery query(database);
    QString allMessagesQuery = "SELECT * FROM " + QString::number(LoginPage::getUser().getId()) + "_chat_" + QString::number(converserId);


    if (query.exec(allMessagesQuery))
    {
        while (query.next())
        {
            if (query.value("sender_id").toString() == QString::number(LoginPage::getUser().getId()))
            {
                ui->chatDisplay->append("<b>Ja</b> (" + query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                ui->chatDisplay->append("<font color=Green>" + query.value("message").toString() + "</font>");
            }
            else
            {
                ui->chatDisplay->append("<b>" + this->windowTitle() + "</b> (" +
                                        query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                ui->chatDisplay->append("<font color=Red>" + query.value("message").toString() + "</font>");
            }

        }
        query.last();
        lastReadMessageId = query.value("message_id").toUInt();

        qDebug() << "ChatWindow::getAllMessagesFromDatabaseAndDisplay() - wykonało się";
        qDebug() << "lastReadMessageId =" << lastReadMessageId;
    }
}

void ChatWindow::sendMessage(quint32 senderId, quint32 receiverId, const QString& message)
{
    QDataStream stream(socket);
    stream << (quint32)senderId << (quint32)receiverId << message;
}

void ChatWindow::closeEvent(QCloseEvent *event)
{
    messagesController->isChatWindowClosed = true;
    //this->deleteLater(); // nie zdaza zwalniac zasobow (polaczenia z baza danych)
    this->~ChatWindow();
}

