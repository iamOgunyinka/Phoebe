#include <QMessageBox>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "buy_book_dialog.hpp"
#include "ui_buy_book_dialog.h"

BuyBookDialog::BuyBookDialog( QList<DatabaseRecordFormat> &&list, QWidget *parent) :
    QDialog(parent), curr_item_index( 0 ), data_list( std::move( list )),
    ui( new Ui::BuyBookDialog )
{
    ui->setupUi( this );
    setMaximumSize( 415, 345 );
    if( data_list.size() == 1 ){
        ui->prevButton->setVisible( false );
        ui->nextButton->setVisible( false );
    } else {
        QObject::connect( ui->prevButton, SIGNAL(clicked(bool)), this, SLOT(onPreviousRecord()) );
        QObject::connect( ui->nextButton, SIGNAL(clicked(bool)), this, SLOT(onNextRecord()) );
    }

    setWindowTitle( "Book purchase" );
    ui->authorLineEdit->setReadOnly( true );
    ui->locationLineEdit->setReadOnly( true );
    ui->publisherLineEdit->setReadOnly( true );
    ui->stockLineEdit->setReadOnly( true );
    ui->titleLineEdit->setReadOnly( true );
    ui->quantityLineEdit->setFocus();

    QObject::connect( ui->quantityLineEdit, SIGNAL(textChanged(QString)), this,
                      SLOT( onQuantityChanged( QString ) ) );
    QObject::connect( ui->purchaseButton, SIGNAL(clicked(bool)), this, SLOT(onItemPurchased()) );
    UpdateNextRecord( curr_item_index );
}

BuyBookDialog::~BuyBookDialog()
{
    delete ui;
}

void BuyBookDialog::onQuantityChanged( QString text )
{
    if( text != old_quantity ){
        old_quantity = text;
        if( !old_quantity.isEmpty() ){
            bool is_valid_quantity = false;
            int quantity = old_quantity.toInt( &is_valid_quantity );
            if( !is_valid_quantity ){
                ui->totalPriceLabel->setText( "Invalid" );
                return;
            }
            ui->totalPriceLabel->setText( tr( "Total: # %1" )
                                          .arg( quantity * data_list[curr_item_index].price ) );
        } else {
            ui->totalPriceLabel->setText( "Total: #0.0k" );
        }
    }
}

void BuyBookDialog::onItemPurchased()
{
    bool is_valid_quantity = false;
    int quantity = ui->quantityLineEdit->text().toInt( &is_valid_quantity );

    if( !is_valid_quantity || quantity <= 0 ){
        ui->quantityLineEdit->setFocus();
        QMessageBox::critical( this, "Error", "Invalid quantity specified" );
        return;
    }

    DatabaseRecordFormat &data = data_list[ curr_item_index ];
    if( static_cast<unsigned int>( quantity ) > data.quantity ){
        ui->quantityLineEdit->setFocus();
        QMessageBox::information( this, "Purchase", "Unfortunately, there are lesser item in stock.");
        return;
    }
    QSqlQuery buy_query {};
    buy_query.prepare( tr( "UPDATE inventory SET stock = %1 WHERE serial_number = %2" )
                       .arg( data.quantity - quantity ).arg( data.serial_number ) );

    if( !buy_query.exec() ){
        qDebug() << buy_query.lastError();
        QMessageBox::critical( this, "Error", "Unable to do purchase, database trouble." );
        return;
    } else {
        data.quantity -= quantity;
        UpdateNextRecord( curr_item_index );
        ui->quantityLineEdit->clear();
        ui->quantityLineEdit->setFocus();
    }

    double const total = quantity * data.price;
    QSqlQuery report_query {};
    report_query.prepare( "INSERT INTO reports( book_title, author_name, stock, price, date_performed, "
                          "transaction_type, total ) VALUES ( :title, :author, :stck, :price, :date, "
                          ":type, :total )");
    report_query.bindValue( ":title", ui->titleLineEdit->text() );
    report_query.bindValue( ":author", ui->authorLineEdit->text() );
    report_query.bindValue( ":stck", quantity );
    report_query.bindValue( ":price", data.price );
    report_query.bindValue( ":total", total );
    report_query.bindValue( ":date", GetDateTime( QDateTime::currentDateTime() ) );
    report_query.bindValue( ":type", static_cast<int>( ReportActionType::SALES ) );

    if( !report_query.exec() ){
        qDebug() << report_query.lastError();
        QMessageBox::information( this, "Report", "Unable to generate report", QMessageBox::Ok );
        return;
    }
    qDebug() << "Report generated and saved.";
    auto res = QMessageBox::information( this, "Purchase", "Item purhased successfully, would you "
                                                           "like to perform another transaction?",
                                         QMessageBox::Yes | QMessageBox::No );
    if( res == QMessageBox::No ){
        accept();
    }
}

void BuyBookDialog::onNextRecord()
{
    if( data_list.size() > 0 && curr_item_index == data_list.size() - 1 ) return;
    ++curr_item_index;
    UpdateNextRecord( curr_item_index );
}

void BuyBookDialog::onPreviousRecord()
{
    if( curr_item_index == 0 ) return;
    --curr_item_index;
    UpdateNextRecord( curr_item_index );
}

void BuyBookDialog::UpdateNextRecord( int pos )
{
    if( pos < 0 || pos >= data_list.size() ) // we're most likely never gonna get here
        return;

    DatabaseRecordFormat const & data = data_list.at( pos );

    ui->dateAdded->setText( "Added on " + data.date_time_added.toString() );
    ui->priceLabel->setText( "Price: N" + QString::number( data.price ) );
    ui->authorLineEdit->setText( data.author_name );
    ui->locationLineEdit->setText( data.location );
    ui->publisherLineEdit->setText( data.publisher );
    ui->stockLineEdit->setText( QString::number( data.quantity ) );
    ui->priceLabel->setText( tr( "Price: #" ) + QString::number( data.price ) );
    ui->titleLineEdit->setText( data.book_title );
    ui->coverImageLabel->clear();
    ui->totalPriceLabel->clear();

    if( !( data.book_cover.isNull() ) ){
        QImage image{ QImage::fromData( data.book_cover ) };

        if( image.isNull() ){
            QMessageBox::warning( this, "View", tr( "Unable to retrieve cover page" ), QMessageBox::Ok );
            return;
        }
        QPixmap cover_image = QPixmap::fromImage( image.scaled( 100, 100, Qt::KeepAspectRatio,
                                                                Qt::SmoothTransformation ) );
        ui->coverImageLabel->setPixmap( cover_image );
        ui->coverImageLabel->setFixedSize( 100, 100 );
    } else {
        ui->coverImageLabel->setText( tr( "NO COVER PAGE"));
    }
}
