#ifndef CONNECTIONLOSTDIALOG_H
#define CONNECTIONLOSTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTcpSocket>
#include <QVBoxLayout>


class ConnectionLostDialog : public QDialog
{
public:
    ConnectionLostDialog(QTcpSocket *socket, QWidget *parent = nullptr);
    QLabel *label;

private slots:
    //void showDialog();

private:
    QTcpSocket *socket;
    QVBoxLayout *layout;

    void centerDialog(QWidget *parent);


};

#endif // CONNECTIONLOSTDIALOG_H
