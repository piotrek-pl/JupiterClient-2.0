#ifndef SIGNUPWINDOW_H
#define SIGNUPWINDOW_H

#include <QMainWindow>

namespace Ui {
class SignUpWindow;
}

class SignUpWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SignUpWindow(QWidget *parent = nullptr);
    ~SignUpWindow();

private:
    Ui::SignUpWindow *ui;
    void closeEvent(QCloseEvent *event);
    bool createUserAccount();
    bool userExists();
    quint32 getUserIdBasedOnUsername(const QString &username);
    void createTablesForTheUser(quint32 userId);
    void createFriendsTable(quint32 userId);

signals:
    void closed();
private slots:
    void on_signUpButton_clicked();
};

#endif // SIGNUPWINDOW_H
