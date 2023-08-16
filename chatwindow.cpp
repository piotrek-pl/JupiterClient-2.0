#include <QMouseEvent>
#include "chatwindow.h"
#include "qscrollbar.h"
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
    setTheMessagesFormat();
    scrollBar = ui->chatDisplay->verticalScrollBar();
    connect(ui->messageInput, &QTextEdit::textChanged, this, &ChatWindow::onTextChanged);
    //this->setWindowTitle(getUsernameAliasFromDatabase(converserId));
    this->setWindowTitle(MainWindow::friendsMap.value(converserId)->getAlias());
    this->converserId = converserId;
    getAllMessagesFromDatabaseAndDisplay();
    socket = MainWindow::getSocket();

    ui->messageInput->installEventFilter(this);
    ui->messageInput->setFocus();

    makeThread();
}


void ChatWindow::setTheMessagesFormat()
{
    incomingMessages.setForeground(QColor::fromRgb(0, 153, 0));
    outgoingMessages.setForeground(Qt::red);
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
            + " WHERE message_id > " + QString::number(lastReadMessageId) + " AND message_id <= "
            + QString::number(lastMessageId);

    qDebug() << unreadMessagesQuery;

    if (query.exec(unreadMessagesQuery))
    {
        while (query.next())
        {
            if (query.value("sender_id").toString() == QString::number(LoginPage::getUser().getId()))
            {
                ui->chatDisplay->append("<b>Ja</b> (" + query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                ui->chatDisplay->setCurrentCharFormat(incomingMessages);
                //ui->chatDisplay->append("<font color=Green>" + query.value("message").toString() + "</font>");
                ui->chatDisplay->append(query.value("message").toString());
            }
            else
            {
                ui->chatDisplay->append("<b>" + this->windowTitle() + "</b> (" +
                                        query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                ui->chatDisplay->setCurrentCharFormat(outgoingMessages);
                //ui->chatDisplay->append("<font color=Red>" + query.value("message").toString() + "</font>");
                ui->chatDisplay->append(query.value("message").toString());
            }

        }
    }
    lastReadMessageId = lastMessageId;

    scrollBar->setValue(scrollBar->maximum());
}

void ChatWindow::onTextChanged()
{
    QString text = ui->messageInput->toPlainText();
    //text = text.trimmed(); // Usunięcie początkowych i końcowych białych znaków (w tym nowych wierszy)

    bool containsOnlySpaces = true;
    for (QChar ch : text) {
        if (!ch.isSpace()) {
            containsOnlySpaces = false;
            break;
        }
    }

    ui->sendButton->setDisabled(containsOnlySpaces);
    //ui->sendButton->setDisabled(ui->messageInput->toPlainText().isEmpty());
}

void ChatWindow::on_sendButton_clicked()
{

    QDataStream stream(socket);
    sendMessage(LoginPage::getUser().getId(), converserId, ui->messageInput->toPlainText());
    ui->messageInput->clear();
}

/*QString ChatWindow::getUsernameAliasFromDatabase(quint32 userId)
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
}*/

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
                //ui->chatDisplay->append("<font color=Green>" + query.value("message").toString() + "</font>");
                ui->chatDisplay->setCurrentCharFormat(incomingMessages);
                ui->chatDisplay->append(query.value("message").toString());
            }
            else
            {
                ui->chatDisplay->append("<b>" + this->windowTitle() + "</b> (" +
                                        query.value("timestamp").toDateTime().toString("dd-MM-yyyy hh:mm:ss)"));
                //ui->chatDisplay->append("<font color=Red>" + query.value("message").toString() + "</font>");
                ui->chatDisplay->setCurrentCharFormat(outgoingMessages);
                ui->chatDisplay->append(query.value("message").toString());
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

bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return && (keyEvent->modifiers() & Qt::AltModifier)) {
            ui->messageInput->insertPlainText("\n");
            return true; // Zdarzenie zostało obsłużone
        }
        else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            ui->sendButton->click(); // Aktywuj przycisk sendButton
            return true; // Zdarzenie zostało obsłużone
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

