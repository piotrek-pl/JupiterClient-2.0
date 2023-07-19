#include <QDebug>
#include <QSqlQuery>
#include <QString>
#include "messagescontroller.h"
#include "loginpage.h"

MessagesController::MessagesController(quint32 converserId, quint32 lastReadMessageId, QObject *parent)
    : QObject{parent},
      converserId{converserId},
      lastReadMessageId{lastReadMessageId}
{
    isChatWindowClosed = false;
    database = QSqlDatabase::addDatabase("QMYSQL", QString("messagesControllerThreadUser%1").arg(converserId));
}

MessagesController::~MessagesController()
{
    database = QSqlDatabase();
    QSqlDatabase::removeDatabase(QString("messagesControllerThreadUser%1").arg(converserId));
    qDebug() << "MessagesController::~MessagesController() - wykonało się";
    qDebug() << "Removed " + QString("messagesControllerThreadUser%1").arg(converserId);
}

void MessagesController::run()
{
    qDebug() << "Jestem w metodzie MessagesController::run()";
    qDebug() << "\tNazwa połączenia z bazą -" << QString("messagesControllerThreadUser%1").arg(converserId);
    checkNewMessages();
}

void MessagesController::checkNewMessages()
{
    QString messageIdQuery = "SELECT message_id FROM " + QString::number(LoginPage::getUser().getId()) + "_chat_" + QString::number(converserId);
    LoginPage::connectToDatabase(database);
    QSqlQuery query(database);

    while(true)
    {
        if (this->isChatWindowClosed)
        {
            this->~MessagesController();
            break;
        }

        if (query.exec(messageIdQuery))
        {
            if (query.size() > 0)
            {
                query.last();
                if (query.value("message_id").toUInt() > lastReadMessageId)
                {
                    emit wroteNewMessageInDatabase(query.value("message_id").toUInt());
                    lastReadMessageId = query.value("message_id").toUInt();
                }
            }
        }
    }
}


