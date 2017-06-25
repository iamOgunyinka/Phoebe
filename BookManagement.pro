#-------------------------------------------------
#
# Project created by QtCreator 2017-06-03T15:36:34
#
#-------------------------------------------------

QT       += core gui sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BookManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        login_dialog.cpp \
    app_main_window.cpp \
    view_inventory_dialog.cpp \
    search_dialog.cpp \
    add_item_dialog.cpp \
    buy_book_dialog.cpp \
    report_dialog.cpp

HEADERS  += login_dialog.hpp \
    app_main_window.hpp \
    view_inventory_dialog.hpp \
    search_dialog.hpp \
    add_item_dialog.hpp \
    buy_book_dialog.hpp \
    report_dialog.hpp \
    resources.hpp

FORMS += \
    inventory_action_dialog.ui \
    view_inventory_dialog.ui \
    buy_book_dialog.ui \
    report_dialog.ui

RESOURCES += \
    res.qrc
