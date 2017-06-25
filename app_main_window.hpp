#ifndef APP_MAIN_WINDOW_HPP
#define APP_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QMdiArea>
#include <QAction>
#include "view_inventory_dialog.hpp"

class AppMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppMainWindow(QWidget *parent = 0);
signals:
private slots:
    void onAddStockActionTriggered();
    void onViewInventoryTriggered();
    void onRemoveStockTriggered();
    void onUpdateStockTriggered();
    void onSearchButtonEntered();
    void onGenerateReportTriggered();
    void onBuyBookActionTriggered();
    void showHelp();
    void onDbOperationCompleted(int);
protected:
    void closeEvent( QCloseEvent *event ) override;
private:
    void SetupDb();
    void CreateActions();
    void CreateMenus();
    void CreateToolbars();
    void CheckForLowStock();
    void AnnounceLowStock();
    QList<DatabaseRecordFormat> PerformTextSearch( QString const & );
private:
    QSqlDatabase db;
    QList<DatabaseRecordFormat> data_list;
    QMdiArea   *workspace;

    QAction *logoutAction;
    QAction *searchAction;
    QAction *viewInventoryAction;
    QAction *addStockAction;
    QAction *removeStockAction;
    QAction *updateStockAction;
    QAction *generateReportAction;
    QAction *buyBookAction;
    QLineEdit *searchEdit;
};

class DBThreadObject : public QObject
{
    Q_OBJECT

public slots:
    void onThreadStarted();
signals:
    void completed( int );
private:
    QSqlDatabase *database;
    QList<DatabaseRecordFormat> &records;
    void SetupDb();
public:
    DBThreadObject( QSqlDatabase *db, QList<DatabaseRecordFormat> & data_list, QObject *parent = nullptr );
};

#endif // APP_MAIN_WINDOW_HPP
