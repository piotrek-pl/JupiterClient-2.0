#include <QMouseEvent>
#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "mainwindow.h"
#include "loginpage.h"

#include <windows.h>

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
    qDebug() << "Jestem w destruktorze ~ChatWindow()";
    delete ui;

    MainWindow::activeChatWindowsMap.remove(converserId);
    MainWindow::friendsMap.value(converserId)->setOpenChatWindow(false);

    if (MainWindow::friendsMap.value(converserId)->isNewMessage())
    {
        MainWindow::changeMessageStatusInTheDatabaseToRead(converserId);
    }

    MainWindow::friendsMap.value(converserId)->setChatWindowOpenedAfterReceivingTheMessage(false);
    qDebug() << "\tsetOpenChatWindow ustawiono na false";
    qDebug() << "\tsetChatWindowOpenedAfterReceivingTheMessage ustawiono na false";
}

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
    //WAŻNA WIADOMOŚĆ - WYŁĄCZONA CHWILOWO
    //qDebug() << "Jestem w metodzie ChatWindow::getAllMessagesFromDatabaseAndDisplay()";

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
        // WAŻNA WIADOMOŚĆ - WYŁĄCZONA CHWILOWO
        //qDebug() << "\tlastReadMessageId =" << lastReadMessageId;
    }
}

void ChatWindow::sendMessage(quint32 senderId, quint32 receiverId, const QString& message)
{
    QDataStream stream(socket);
    stream << (quint32)senderId << (quint32)receiverId << message;
}

void ChatWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    messagesController->isChatWindowClosed = true;

    //this->deleteLater(); // nie zdaza zwalniac zasobow (polaczenia z baza danych)
    this->~ChatWindow();
}

// dodaj jeszcze klikniecie na kazdy z elementow
/*void ChatWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    qDebug() << "nacisnieto przycisk myszy";
    // zmienic stan wiadomosci jak ponizej, tyle ze za duzo sie dzieje
    // i jak nie ma wiadomosci, a klikam tez probuje zmienic
    MainWindow::changeMessageStatusInTheDatabaseToRead(converserId);
    this->setWindowIcon(QIcon());

    //QMainWindow::mousePressEvent(event);

}*/

bool ChatWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    MSG* msg = static_cast<MSG*>(message);

    if ((msg->message == WM_LBUTTONDOWN ||
         msg->message == WM_RBUTTONDOWN ||
         msg->message == WM_NCLBUTTONDOWN ||
         msg->message == WM_NCRBUTTONDOWN) &&
        (msg->wParam != HTCLOSE && msg->wParam != SC_CLOSE))
    {
        qDebug() << MainWindow::friendsMap.value(converserId)->isNewMessage();
        if (MainWindow::friendsMap.value(converserId)->isNewMessage())
        {
            MainWindow::changeMessageStatusInTheDatabaseToRead(converserId);
            //this->setWindowIcon(QIcon());
        }
    }

    return false;
}



