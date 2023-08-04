#include <QThread>
#include "invitationcontroller.h"
#include "mainwindow.h"
#include "loginpage.h"

InvitationController::InvitationController(QObject *parent)
    : QObject{parent}
{
    database = QSqlDatabase::addDatabase("QMYSQL", "invitationControllerThread");
    LoginPage::connectToDatabase(database);
}

void InvitationController::run()
{
    checkInvitations();
}

void InvitationController::checkInvitations()
{
    QList<Invitation *> sentInvivationsList = MainWindow::sentInvitationsList;
    QList<Invitation *> receivedInvitationsList = MainWindow::receivedInvitationsList;

    QList<Invitation *> currentSentInvitationsList = getInvitationsList("sent");
    QList<Invitation *> currentReceivedInvitationsList = getInvitationsList("received");

    while (true)
    {
        //qDebug() << "Jestem w invitationcontroller";

        bool areSentInvitationsEqual = true;
        areSentInvitationsEqual = true;
        if (currentSentInvitationsList.size() == sentInvivationsList.size())
        {
            for (int i = 0; i < currentSentInvitationsList.size(); ++i)
            {
                if (*currentSentInvitationsList.at(i) != *sentInvivationsList.at(i))
                {
                    areSentInvitationsEqual = false;
                    break;
                }
            }
        }
        else
        {
            areSentInvitationsEqual = false;
        }

        if (!areSentInvitationsEqual)
        {
            //qDebug() << "\tListy są różne.";
            emit sentInvitationsChanged();
        }
        else
        {
            //qDebug() << "\tListy są takie same.";
        }

        sentInvivationsList = currentSentInvitationsList;
        currentSentInvitationsList = getInvitationsList("sent");

        bool areReceivedInvitationsEqual = true;
        if (currentReceivedInvitationsList.size() == receivedInvitationsList.size())
        {
            for (int i = 0; i < currentReceivedInvitationsList.size(); ++i)
            {
                if (*currentReceivedInvitationsList.at(i) != *receivedInvitationsList.at(i))
                {
                    areReceivedInvitationsEqual = false;
                    break;
                }
            }
        }
        else
        {
            areReceivedInvitationsEqual = false;
        }

        if (!areReceivedInvitationsEqual)
        {
            //qDebug() << "\tListy są różne.";
            emit receivedInvitationsChanged();
        }
        else
        {
            //qDebug() << "\tListy są takie same.";
        }

        receivedInvitationsList = currentReceivedInvitationsList;
        currentReceivedInvitationsList = getInvitationsList("received");

        QThread::sleep(1);
    }
}

QList<Invitation *> InvitationController::getInvitationsList(QString invitationType)
{
    QList<Invitation *> invitationsList;
    QString invitations = QString("SELECT %1_%2_invitations.invitation_id, "
                                             "%1_%2_invitations.id, "
                                             "%1_%2_invitations.username "
                                             "FROM %1_%2_invitations")
                                        .arg(LoginPage::getUser().getId())
                                        .arg(invitationType);

    QSqlQuery query(database);
    if (query.exec(invitations))
    {
        if (query.numRowsAffected() > 0)
        {
            while (query.next())
            {
                invitationsList.append(new Invitation(query.value("invitation_id").toUInt(),
                                                          query.value("id").toUInt(),
                                                          query.value("username").toString()));
            }
        }
    }

    return invitationsList;
}
