#include "connectionlostdialog.h"

#include <QMainWindow>
#include <QMoveEvent>

ConnectionLostDialog::ConnectionLostDialog(QWidget *parent)
    : QDialog(parent)
{
    //setWindowTitle("Connection Lost");
    //setGeometry(0, 200, 300, 100);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    label = new QLabel("Connection to the server has been lost.", this);
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);

    if (parent)
    {
        centerDialog(parent);
    }

    //setWindowModality(Qt::WindowModal);

    //connect(socket, &QTcpSocket::disconnected, this, &ConnectionLostDialog::showDialog);
    //connect(socket, &QTcpSocket::connected, this, &ConnectionLostDialog::close);
}

void ConnectionLostDialog::centerDialog(QWidget *parent)
{
    int xOffset = 222;
    QPoint parentCenter = parent->geometry().center();
    int x = parentCenter.x() - width() / 2 - xOffset;
    int y = parentCenter.y() - height() / 2;
    move(x, y);
}
