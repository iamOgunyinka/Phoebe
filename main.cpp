#include "login_dialog.hpp"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginDialog w;
    w.show();

    return a.exec();
}
