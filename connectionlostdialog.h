#ifndef CONNECTIONLOSTDIALOG_H
#define CONNECTIONLOSTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTcpSocket>
#include <QVBoxLayout>


class ConnectionLostDialog : public QDialog
{
public:
    ConnectionLostDialog(QWidget *parent = nullptr);
    QLabel *label;

private slots:
    //void showDialog();

private:
    QVBoxLayout *layout;

    void centerDialog(QWidget *parent);


};

#endif // CONNECTIONLOSTDIALOG_H
