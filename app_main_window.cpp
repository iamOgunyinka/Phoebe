#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrinter>
#include <QSqlError>
#include <QSqlQuery>
#include <QStatusBar>
#include <QTextDocument>
#include <QThread>
#include <QToolBar>
#include "add_item_dialog.hpp"
#include "app_main_window.hpp"
#include "buy_book_dialog.hpp"
#include "search_dialog.hpp"
#include "report_dialog.hpp"

AppMainWindow::AppMainWindow(QWidget *parent) : QMainWindow(parent),
    workspace( new QMdiArea )
{
    setAttribute( Qt::WA_DeleteOnClose );
    setWindowTitle( tr( "Main Menu" ) );
    workspace->setBackground( QBrush( QImage( ":/new/icons/icons/inventory_logo.png" ) ) );
    setCentralWidget( workspace );
    setWindowState( Qt::WindowMaximized );
    setWindowIcon( QIcon( ":/new/icons/icons/logo.png") );

    CreateActions();
    CreateMenus();
    CreateToolbars();

    SetupDb();
}

void AppMainWindow::SetupDb()
{
    QThread *db_thread = new QThread( this );
    DBThreadObject *thread_object = new DBThreadObject( &db, data_list );
    thread_object->moveToThread( db_thread );
    QObject::connect( db_thread, SIGNAL(started()), thread_object, SLOT(onThreadStarted()) );
    QObject::connect( thread_object, SIGNAL(completed(int)), this, SLOT( onDbOperationCompleted(int)));
    QObject::connect( thread_object, SIGNAL(completed(int)), db_thread, SLOT(quit()) );
    QObject::connect( db_thread, SIGNAL(finished()), db_thread, SLOT(deleteLater()) );
    db_thread->start();
}

void AppMainWindow::AnnounceLowStock()
{
    if( !data_list.isEmpty() ){
        QString alert_string{ "The following books are getting low in stock\r\n\r\n" };
        for( auto const & str : data_list ){
            alert_string += str.book_title + tr( " by %1 ( %2 left )\r\n" ).arg( str.author_name )
                    .arg( str.quantity );
        }
        QMessageBox::information( this, "Low stock", alert_string, QMessageBox::Ok );
    }
    data_list.clear();
}

void AppMainWindow::onDbOperationCompleted( int status )
{
    if(status != 0 ){
        QMessageBox::critical( this, "Database error", "Something is wrong with the database", QMessageBox::Ok);
        std::exit( status );
    }
    AnnounceLowStock();
    this->statusBar()->showMessage( "Done" );
}

/// create action objects that carry out our operations, set their shortcuts etc
void AppMainWindow::CreateActions()
{
    logoutAction = new QAction( QIcon( ":/new/icons/icons/logout.png"), tr( "Log off") );
    logoutAction->setShortcut( tr( "Ctrl+Q" ));
    logoutAction->setStatusTip( tr( "Logs you off and close this main window"));
    QObject::connect( logoutAction, SIGNAL(triggered(bool)), this, SLOT( close()) );

    buyBookAction = new QAction( QIcon(":/new/icons/icons/buy.png"), "Buy book" );
    buyBookAction->setShortcut( tr( "Ctrl+Y" ) );
    buyBookAction->setStatusTip( tr( "Buy book" ) );
    QObject::connect( buyBookAction, SIGNAL(triggered(bool)), this, SLOT( onBuyBookActionTriggered() ));

    searchAction = new QAction( QIcon( ":/new/icons/icons/search.png"), tr( "Search") );
    searchAction->setShortcut( tr( "Ctrl+F") );
    searchAction->setStatusTip( tr( "Search records for corrresponding book(s).") );

    addStockAction = new QAction( QIcon( ":/new/icons/icons/add.png" ), tr( "Add Stock") );
    addStockAction->setShortcut( tr( "Ctrl+N" ) );
    addStockAction->setStatusTip( tr( "Add new book to the inventory." ) );
    QObject::connect( addStockAction, SIGNAL(triggered(bool)), this, SLOT( onAddStockActionTriggered()) );

    viewInventoryAction = new QAction( QIcon( ":/new/icons/icons/about.png"), tr( "View all Records") );
    viewInventoryAction->setShortcut( tr( "Ctrl+O" ) );
    viewInventoryAction->setStatusTip( tr( "Show all available records" ));
    QObject::connect( viewInventoryAction, SIGNAL(triggered(bool)), this, SLOT(onViewInventoryTriggered()) );

    removeStockAction = new QAction( QIcon( ":/new/icons/icons/remove.png" ), tr( "Remove" ) );
    removeStockAction->setShortcut( tr( "Ctrl+D") );
    removeStockAction->setStatusTip( tr( "Remove book(s)from the inventory.") );
    QObject::connect( removeStockAction, SIGNAL(triggered(bool)), this, SLOT( onRemoveStockTriggered()) );

    updateStockAction = new QAction( QIcon( ":/new/icons/icons/update.png" ), tr( "Update" ) );
    updateStockAction->setShortcut( tr( "Ctrl+V") );
    updateStockAction->setStatusTip( tr( "Edit records") );
    QObject::connect( updateStockAction, SIGNAL(triggered(bool)), this, SLOT(onUpdateStockTriggered()) );

    generateReportAction = new QAction( QIcon(":/new/icons/icons/report.png"), "Generate Report" );
    generateReportAction->setShortcut( tr("Ctrl+G"));
    generateReportAction->setStatusTip( "Generate all reports on inventory");
    QObject::connect( generateReportAction, SIGNAL(triggered(bool)), this, SLOT(onGenerateReportTriggered()) );
}

void AppMainWindow::showHelp()
{
    QString const help = "<b>This is a book inventory utility system</b><br><br>"
                         "To <b>add new</b> record(s) to the inventory, simply use the command Ctrl + N<br>"
                         "or navigate to <b>Actions</b>, then choose <b>Add New</b>.<br><br>"
                         "To <b>View All</b> the records, simply use the shortcut Ctrl + O<br><br>"
                         "To <b>Update Record(s)</b>, use the pencil on the toolbar. It then open a"
                         " new window asking for a ( part of the ) title for the book or the author's name"
                         " or both. If results are found, you may then continue to update all the records"
                         " corresponding to the new data. Then finally, click on <b>Update</b><br><br>"
                         "To <b>Remove</b> record(s) from the inventory, use the Ctrl + D combination, "
                         "type the author or book name to the search dialog and proceed into deleting"
                         " the record(s) that you want.";
    QMessageBox::information( this, "Help", help, QMessageBox::Ok );
}

void AppMainWindow::CreateMenus()
{
    QAction *helpAction = new QAction{ QIcon( ":/new/icons/icons/help.png" ), tr( "Help" ) };
    helpAction->setShortcut( tr("F1"));
    QObject::connect( helpAction, SIGNAL(triggered(bool)), this, SLOT( showHelp() ) );

    QMenu *fileMenu, *actionsMenu;
    fileMenu = this->menuBar()->addMenu( tr( "File" ) );
    fileMenu->addAction( helpAction );
    fileMenu->addAction( generateReportAction );
    fileMenu->addAction( logoutAction );

    actionsMenu = this->menuBar()->addMenu( tr( "Actions" ) );
    actionsMenu->addAction( buyBookAction );
    actionsMenu->addAction( addStockAction );
    actionsMenu->addAction( viewInventoryAction );
    actionsMenu->addAction( searchAction );
    actionsMenu->addAction( removeStockAction );
    actionsMenu->addAction( updateStockAction );
}

void AppMainWindow::CreateToolbars()
{
    QToolBar *toolbar = this->addToolBar( "Main" );
    QToolBar *searchToolbar = this->addToolBar( "Testing");

    searchEdit = new QLineEdit();
    searchEdit->setStatusTip( tr( "Search inventory for specific book(s)." ));
    searchEdit->setFixedWidth( 200 );

    searchToolbar->setLayoutDirection( Qt::RightToLeft );
    searchToolbar->addWidget( searchEdit );

    toolbar->addAction( buyBookAction );
    toolbar->addSeparator();
    toolbar->addAction( addStockAction );
    toolbar->addSeparator();
    toolbar->addAction( viewInventoryAction );
    toolbar->addSeparator();
    toolbar->addAction( generateReportAction );
    toolbar->addSeparator();
    toolbar->addAction( updateStockAction );
    toolbar->addSeparator();
    toolbar->addAction( removeStockAction );

    QObject::connect( searchAction, SIGNAL( triggered( bool ) ), searchEdit, SLOT( setFocus() ) );
    QObject::connect( searchEdit, SIGNAL( returnPressed() ), this, SLOT( onSearchButtonEntered() ) );
}

void AppMainWindow::closeEvent( QCloseEvent *event )
{
    if( QMessageBox::information( this, tr( "Log out"), tr( "Are you sure you want to log out?"),
                              QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
    {
        workspace->closeAllSubWindows();
        event->accept();
    } else {
        event->ignore();
    }
}

void AppMainWindow::onAddStockActionTriggered()
{
    AddItemDialog *dialog { new AddItemDialog( this ) };
    dialog->exec();
}

void AppMainWindow::onSearchButtonEntered()
{
    searchEdit->clearFocus();
    auto data_list = PerformTextSearch( searchEdit->text() );
    if( data_list.isEmpty() ){
        QMessageBox::information( this, "Search", tr( "No result found" ), QMessageBox::Ok );
        return;
    }

    ViewInventoryDialog *inventoryDialog = new ViewInventoryDialog( ActionType::View, this );
    inventoryDialog->SetDataList( std::move( data_list ));
    inventoryDialog->exec();
}

QList<DatabaseRecordFormat> AppMainWindow::PerformTextSearch( QString const & text )
{
    SearchDialog *searchDialog = new SearchDialog( text, this );
    if( searchDialog->exec() != QDialog::Accepted ) return {};

    QSqlQuery searchQuery;
    searchQuery.prepare( tr( "SELECT * FROM inventory WHERE MATCH ( book_title, author_name ) "
                         "AGAINST ( ' %1 %2 ' IN NATURAL LANGUAGE MODE )" )
                         .arg( searchDialog->GetBookTitle() )
                         .arg( searchDialog->GetAuthorName() ) );

    if( !searchQuery.exec() ){
        qDebug() << searchQuery.lastError();
        QMessageBox::warning( this, "Search", "There was a problem executing the instructions "
                                              "necessary for the search", QMessageBox::Ok );
        return {};
    }
    QList<DatabaseRecordFormat> data_list {};
    FillRecordFromQuery( data_list, searchQuery );
    return data_list;
}

void AppMainWindow::onViewInventoryTriggered()
{
    ViewInventoryDialog *allRecordsDialog { new ViewInventoryDialog( ActionType::View, this ) };
    allRecordsDialog->CheckDatabaseRecord();
    allRecordsDialog->exec();
}

void AppMainWindow::onRemoveStockTriggered()
{
    data_list = PerformTextSearch( "" );
    if( data_list.isEmpty() ){
        QMessageBox::information( this, "Remove", "No record found", QMessageBox::Ok );
        return;
    }
    ViewInventoryDialog *deleteDialog = new ViewInventoryDialog( ActionType::Delete, this );
    deleteDialog->SetDataList( std::move( data_list ));
    deleteDialog->exec();
}

void AppMainWindow::onUpdateStockTriggered()
{
    data_list = PerformTextSearch( "" );
    if( data_list.isEmpty() ){
        QMessageBox::information( this, "Update", "No record found", QMessageBox::Ok );
        return;
    }
    ViewInventoryDialog *updateDialog = new ViewInventoryDialog( ActionType::Update, this );
    updateDialog->SetDataList( std::move( data_list ));
    updateDialog->exec();
    CheckForLowStock();
}

void AppMainWindow::CheckForLowStock()
{
    QSqlQuery alert_query( "SELECT * FROM inventory WHERE stock < 5" );
    if( !alert_query.exec() ){
        qDebug() << alert_query.lastError();
        return;
    }

    FillRecordFromQuery( data_list, alert_query );
    AnnounceLowStock();
}

void AppMainWindow::onGenerateReportTriggered()
{
    ReportDialog *report_dialog = new ReportDialog( this );
    report_dialog->exec();
}

DBThreadObject::DBThreadObject(QSqlDatabase *db, QList<DatabaseRecordFormat> &list, QObject *parent )
    : QObject( parent ), database{ db }, records{ list }{
}

void DBThreadObject::onThreadStarted()
{
    SetupDb();
}

void DBThreadObject::SetupDb()
{
    // setting up the database, we use 'localhost' since we're running the code on our local machine
    *database = QSqlDatabase::addDatabase( "QMYSQL" );
    database->setHostName( tr( "localhost" ) );
    database->setDatabaseName( tr( "debug_db" ) );
    database->setUserName( tr( "iamScope" ) );
    database->setPassword( tr( "scope" ) );

    // we open the connection to the db, if it fails, then we have nothing to work with --> quit!
    if( !database->open() ){
        qDebug() << database->lastError();
        emit completed( -1 );
        return;
    }

    // create the database's table we need
    QSqlQuery create_table_query {};
    create_table_query.prepare( tr( "CREATE TABLE IF NOT EXISTS %1 ( "
                                    "serial_number INTEGER AUTO_INCREMENT PRIMARY KEY, "
                                    "book_title TEXT,"
                                    "author_name TEXT,"
                                    "publisher TEXT, date_time DATETIME, "
                                    "stock INTEGER NOT NULL, price DOUBLE, "
                                    "location TEXT, "
                                    "book_cover BLOB, FULLTEXT( book_title, author_name ) "
                                    ") ENGINE=InnoDB"
                                    "" ).arg( AddItemDialog::TABLE_NAME ));
    // we execute the query to create the table, if it fails, we quit!
    if( !create_table_query.exec() ){
        qDebug() << database->lastError();
        emit completed( -1 );
        return;
    }

    QSqlQuery report_query {};
    report_query.prepare( tr( "CREATE TABLE IF NOT EXISTS reports ( "
                              "serial_number INTEGER AUTO_INCREMENT PRIMARY KEY,"
                              "book_title TEXT, author_name TEXT, stock INTEGER NOT NULL, "
                              "price DOUBLE, total DOUBLE, date_performed DATETIME, "
                              "transaction_type INTEGER ) "));

    if( !report_query.exec() ){
        qDebug() << database->lastError();
        emit completed( -1 );
        return;
    }

    QSqlQuery alert_query{ "SELECT * FROM inventory WHERE stock < 5" };
    if( !alert_query.exec() ){
        qDebug() << alert_query.lastError();
        emit completed( -1 );
    }

    FillRecordFromQuery( records, alert_query );
    emit completed( 0 );
}

void AppMainWindow::onBuyBookActionTriggered()
{
    auto list = PerformTextSearch( "" );
    if( list.isEmpty() ){
        QMessageBox::information( this, "Purchase", "No item found" );
        return;
    }
    BuyBookDialog *buy_book_dialog = new BuyBookDialog( std::move( list ), this );
    buy_book_dialog->exec();
    CheckForLowStock();
}
