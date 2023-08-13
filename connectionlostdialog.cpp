#include "connectionlostdialog.h"

#include <QMainWindow>
#include <QMoveEvent>

ConnectionLostDialog::ConnectionLostDialog(QTcpSocket *socket, QWidget *parent)
    : QDialog(parent), socket(socket)
{
    //setWindowTitle("Connection Lost");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    label = new QLabel("Connection to the server has been lost.", this);
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);

    //setWindowModality(Qt::WindowModal);

    //connect(socket, &QTcpSocket::disconnected, this, &ConnectionLostDialog::showDialog);
    //connect(socket, &QTcpSocket::connected, this, &ConnectionLostDialog::close);
}
