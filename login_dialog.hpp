#ifndef LOGIN_DIALOG_HPP
#define LOGIN_DIALOG_HPP

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class LoginDialog : public QDialog
{
    Q_OBJECT
private:
    QPushButton *loginButton;
    QPushButton *cancelButton;
    QLineEdit   *passwordEdit;
    QLabel      *passwordLabel;

    QString     passwordText;
private:
    std::string encodeDecode( std::string const & );
    void        onLoginSuccessful();
private slots:
    void        loginClicked();
public:
    LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
};

#endif // LOGIN_DIALOG_HPP
