#include "view_inventory_dialog.hpp"
#include "ui_view_inventory_dialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QImageReader>
#include <QFileDialog>
#include <QBuffer>
#include <QImageWriter>

ViewInventoryDialog::ViewInventoryDialog( ActionType action, QWidget *parent) :
    QDialog( parent ),
    ui( new Ui::ViewInventoryDialog ), curr_record_index( 0 )
{
    ui->setupUi(this);
    setMaximumSize( 400, 350 );

    switch( action )
    {
    case ActionType::Delete:
        SetupWindowForDelete();
        break;
    case ActionType::Update:
        SetupWindowForUpdate();
        break;
    default:
        ui->actionButton->setVisible( false );
        ui->uploadButton->setVisible( false );
    }

    ui->dateAddedLineEdit->setReadOnly( true );

    QObject::connect( ui->nextButton, SIGNAL( clicked( bool ) ), this, SLOT( onNextRecord()) );
    QObject::connect( ui->prevButton, SIGNAL( clicked( bool ) ), this, SLOT( onPreviousRecord() ) );
}

ViewInventoryDialog::~ViewInventoryDialog()
{
    delete ui;
}

void ViewInventoryDialog::SetupWindowForDelete()
{
    ui->actionButton->setText( "&Delete" );
    ui->actionButton->setVisible( true );
    ui->uploadButton->setVisible( false );

    QObject::connect( ui->actionButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteButtonClicked()) );
}

void ViewInventoryDialog::SetupWindowForUpdate()
{
    ui->actionButton->setText( "&Update" );
    ui->actionButton->setVisible( true );

    ui->authorLineEdit->setReadOnly( false );
    ui->locationLineEdit->setReadOnly( false );
    ui->publisherLineEdit->setReadOnly( false );
    ui->stockLineEdit->setReadOnly( false );
    ui->priceLineEdit->setReadOnly( false );
    ui->titleLineEdit->setReadOnly( false );

    QObject::connect( ui->actionButton, SIGNAL(clicked(bool)), this, SLOT( onUpdateButtonClicked()) );
    QObject::connect( ui->uploadButton, SIGNAL( clicked(bool)), this, SLOT( onUploadButtonClicked()) );
}

void ViewInventoryDialog::onUploadButtonClicked()
{
    QString filename = QFileDialog::getOpenFileName( this, tr( "Open picture" ), QString(),
                                                     tr("PNG Images(*.png)"));
    if( !filename.isNull() ){
        QImage image{ filename };
        if( image.isNull() ){
            QMessageBox::warning( this, "Open picture", "Unable to open picture", QMessageBox::Ok );
            return;
        }
        image = image.scaled( 100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        ui->coverLabel->clear();
        ui->coverLabel->setPixmap( QPixmap::fromImage( image ));
        ui->coverLabel->setMaximumSize( QSize( 100, 100 ) );
        m_image = image;
    }
}

void ViewInventoryDialog::onDeleteButtonClicked()
{
    if( QMessageBox::warning( this, "Delete", "Are you sure you want to delete this information?",
                              QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
        return;

    QSqlQuery deleteQuery {};
    unsigned int id = data_list.at( curr_record_index ).serial_number;

    deleteQuery.prepare( "DELETE FROM inventory WHERE serial_number = " + QString::number( id ) );
    if( !deleteQuery.exec() ){
        qDebug() << deleteQuery.lastError();
        QMessageBox::critical( this, "Delete", "Unable to delete record", QMessageBox::Ok );
        return;
    }
    GenerateReport( ActionType::Delete );
    data_list.removeAt( curr_record_index );
    if( data_list.isEmpty() ){
        QMessageBox::information( this, "Inventory", "Empty records", QMessageBox::Ok );
        accept();
        return;
    }
    curr_record_index = 0;
    UpdateNextRecord( curr_record_index );
}

void ViewInventoryDialog::onUpdateButtonClicked()
{
    unsigned int const serial_number = data_list.at( curr_record_index ).serial_number;

    auto is_valid_quantity = false, is_valid_price = false;

    int const stock = ui->stockLineEdit->text().toInt( &is_valid_quantity );
    double const price = ui->priceLineEdit->text().toDouble( &is_valid_price );

    if( !is_valid_quantity || stock <= 0 ){
        QMessageBox::critical( this, "Update", tr( "Invalid quantity specified."));
        return;
    }

    if( !is_valid_price || price <= 0.0 ){
        QMessageBox::critical( this, "Update", tr( "Invalid price specified."));
        return;
    }

    QSqlQuery updateQuery {};
    updateQuery.prepare( tr( "UPDATE inventory SET book_title = :title , author_name = :author,"
                             " publisher = :publisher, stock = :stock, price = :price, "
                             "location = :location, book_cover = :cover WHERE serial_number = " )
                         + QString::number( serial_number ) );

    updateQuery.bindValue( ":title", ui->titleLineEdit->text() );
    updateQuery.bindValue( ":author", ui->authorLineEdit->text() );
    updateQuery.bindValue( ":publisher", ui->publisherLineEdit->text() );
    updateQuery.bindValue( ":stock", stock );
    updateQuery.bindValue( ":price", price );
    updateQuery.bindValue( ":location", ui->locationLineEdit->text() );

    {
        QBuffer buffer {};
        QImageWriter image_writer{ &buffer, "PNG" };
        image_writer.write( m_image );

        updateQuery.bindValue( ":cover", buffer.data() );
    }

    if( !updateQuery.exec() ){
        qDebug() << updateQuery.lastError();
        QMessageBox::warning( this, "Update", "Unable to update data", QMessageBox::Ok );
        return;
    }

    GenerateReport( ActionType::Update );
    QMessageBox::information( this, "Update", "Information updated successfully", QMessageBox::Ok );
}

void ViewInventoryDialog::onNextRecord()
{
    if( data_list.size() > 0 && curr_record_index == data_list.size() - 1 ) return;
    ++curr_record_index;
    UpdateNextRecord( curr_record_index );
}

void ViewInventoryDialog::onPreviousRecord()
{
    if( curr_record_index == 0 ) return;
    --curr_record_index;
    UpdateNextRecord( curr_record_index );
}

void ViewInventoryDialog::CheckDatabaseRecord()
{
    // since we're maintaining a single database connection, Qt knows what DB to call this on
    QSqlQuery select_query { tr( "SELECT * FROM inventory" ) };
    if( !select_query.exec() ){
        qDebug() << select_query.lastError();
        QMessageBox::critical( this, "View", tr( "Unable to retrieve any information from the inventory"),
                               QMessageBox::Ok );
        accept();
    }
    FillRecordFromQuery( data_list, select_query );
    if( data_list.isEmpty() ){
        QMessageBox::information( this, "View", tr( "Nothing has been saved in the inventory yet" ),
                                  QMessageBox::Ok );
        accept();
    } else {
        UpdateNextRecord( 0 );
    }
}

void ViewInventoryDialog::UpdateNextRecord( int pos )
{
    if( pos < 0 || pos >= data_list.size() ) // we're most likely never gonna get here
        return;

    DatabaseRecordFormat const & data = data_list.at( pos );

    ui->dateAddedLineEdit->setText( data.date_time_added.toString() );
    ui->authorLineEdit->setText( data.author_name );
    ui->locationLineEdit->setText( data.location );
    ui->publisherLineEdit->setText( data.publisher );
    ui->stockLineEdit->setText( QString::number( data.quantity ) );
    ui->titleLineEdit->setText( data.book_title );
    ui->priceLineEdit->setText( QString::number( data.price ) );
    ui->coverLabel->clear();

    if( !( data.book_cover.isNull() ) ){
        QImage image{ QImage::fromData( data.book_cover ) };

        if( image.isNull() ){
            QMessageBox::warning( this, "View", tr( "Unable to retrieve cover page" ), QMessageBox::Ok );
            return;
        }
        QPixmap cover_image = QPixmap::fromImage( image.scaled( 100, 100, Qt::KeepAspectRatio,
                                                                Qt::SmoothTransformation ) );
        ui->coverLabel->setPixmap( cover_image );
        ui->coverLabel->setFixedSize( 100, 100 );
        m_image = image;
    } else {
        ui->coverLabel->setText( tr( "NO COVER PAGE"));
        m_image = QImage();
    }
}

void ViewInventoryDialog::SetDataList( QList<DatabaseRecordFormat> && list )
{
    data_list.clear();
    data_list = std::move( list );
    curr_record_index = 0;
    UpdateNextRecord( curr_record_index );
}

void ViewInventoryDialog::GenerateReport( ActionType action )
{
    QSqlQuery delete_report_query{};
    delete_report_query.prepare( "INSERT INTO reports( book_title, author_name, stock, price, "
                                 "date_performed, transaction_type, total ) VALUES ( :title, :author, "
                                 ":stck, :price, :date, :type, :total )");
    delete_report_query.bindValue( ":title", ui->titleLineEdit->text() );
    delete_report_query.bindValue( ":author", ui->authorLineEdit->text() );
    delete_report_query.bindValue( ":date", GetDateTime( QDateTime::currentDateTime() ) );
    delete_report_query.bindValue( ":price", ui->priceLineEdit->text() );
    delete_report_query.bindValue( ":total", 0.0 );

    if( action == ActionType::Delete ){
        delete_report_query.bindValue( ":type", static_cast<int>( ReportActionType::DELETIONS ) );
        delete_report_query.bindValue( ":stck", ui->stockLineEdit->text().toInt() );
    } else if ( action == ActionType::Update ){
        delete_report_query.bindValue( ":type", static_cast<int>( ReportActionType::UPDATES ) );
        int initial_stock_avaiable = data_list.at( curr_record_index ).quantity;
        int new_stock_available = ui->stockLineEdit->text().toInt();
        if( initial_stock_avaiable != new_stock_available ){
            int stock_quantity = new_stock_available > initial_stock_avaiable?
                        new_stock_available - initial_stock_avaiable:
                        initial_stock_avaiable - new_stock_available;
            delete_report_query.bindValue( ":stck", stock_quantity );
        } else {
            delete_report_query.bindValue( ":stck", ui->stockLineEdit->text().toInt() );
        }
    }

    if( !delete_report_query.exec() ){
        qDebug() << delete_report_query.lastError();
        QMessageBox::information( this, "Report", "Unable to generate report", QMessageBox::Ok );
        return;
    }
}
