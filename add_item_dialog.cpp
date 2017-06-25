#include "add_item_dialog.hpp"
#include "ui_inventory_action_dialog.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCloseEvent>
#include <QFileDialog>
#include <QBuffer>
#include <QImageWriter>
#include <QDebug>
#include "resources.hpp"

AddItemDialog::AddItemDialog( QWidget *parent) :
    QDialog(parent), ui( new Ui::InventoryActionDialog ),
    cover_page_used{ false }
{
    ui->setupUi( this );

    QObject::connect( ui->saveButton, SIGNAL(clicked(bool)), this, SLOT( onSaveButtonClicked()) );
    QObject::connect( ui->cancelButton, SIGNAL( clicked(bool)), this, SLOT( close()) );
    QObject::connect( ui->uploadButton, SIGNAL(clicked(bool)), this, SLOT(onUploadButtonClicked()) );

    setMaximumSize( QSize( 400, 350 ) );
    setWindowTitle( tr( "Add New record" ));
    ui->dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
}

QString AddItemDialog::TABLE_NAME = "inventory";

AddItemDialog::~AddItemDialog()
{
    delete ui;
}

bool AddItemDialog::IsColumnsEmpty()
{
    return ( ui->authorLineEdit->text().isEmpty() || ui->titleLineEdit->text().isEmpty() ||
             ui->locationLineEdit->text().isEmpty() || ui->stockLineEdit->text().isEmpty() ||
             ui->priceLineEdit->text().isEmpty() || ui->publisherLineEdit->text().isEmpty() );
}

void AddItemDialog::onSaveButtonClicked()
{
    if( IsColumnsEmpty() )
    {
        QMessageBox::warning( this, tr("Save"), tr("Please make sure all proper boxes are filled"),
                              QMessageBox::Ok );
        return;
    }
    bool is_valid_quantity = false, is_valid_price = false;
    int const quantity = ui->stockLineEdit->text().toInt( &is_valid_quantity );

    if( !is_valid_quantity || quantity <= 0 ){
        QMessageBox::warning( this, "Save", tr( "Invalid stock number" ), QMessageBox::Ok );
        return;
    }

    double const price = ui->priceLineEdit->text().toDouble( &is_valid_price );
    if( !is_valid_price || price <= 0.0 ){
        QMessageBox::warning( this, "Save", tr( "Invalid price used" ) );
        return;
    }

    QString const columns = tr( "date_time, book_title, author_name, publisher, stock, price, location" ) +
                    ( cover_page_used == true ? tr( ", book_cover" ) : tr( "" ) );
    QString const values = tr( "'" ) + GetDateTime( ui->dateTimeEdit->dateTime() ) + "', "
            + tr( "'" ) + ui->titleLineEdit->text() + "', '"
            + ui->authorLineEdit->text() + "', '"
            + ui->publisherLineEdit->text() + "', '"
            + ui->stockLineEdit->text() + "', '"
            + ui->priceLineEdit->text() + "', '"
            + ui->locationLineEdit->text() + tr( "'" );

    QString const insert_string = tr( "INSERT INTO %1 ( %2 ) VALUES( %4 %3 )")
            .arg( TABLE_NAME ).arg( columns )
            .arg( cover_page_used == true ? ", :image_data" : "" ).arg( values );

    QSqlQuery query{};
    query.prepare( insert_string );

    if( cover_page_used && !m_cover.isNull() ){
        QBuffer buffer {};
        QImageWriter image_writer{ &buffer, "PNG" };
        image_writer.write( m_cover );

        query.bindValue( ":image_data", buffer.data() );
    }

    if( !query.exec() ){
        QMessageBox::warning( this, "Save", query.lastError().text(), QMessageBox::Ok );
    } else {
        GenerateReport();
        if( QMessageBox::information( this, "Save",
                                      tr("Information saved successfully, would you like to add more?" ),
                                      QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
        {
            ClearEntries();
            this->accept();
            return;
        }
        ClearEntries();
    }
}

void AddItemDialog::ClearEntries()
{
    ui->dateTimeEdit->clear();
    ui->authorLineEdit->clear();
    ui->locationLineEdit->clear();
    ui->publisherLineEdit->clear();
    ui->stockLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->titleLineEdit->clear();
    ui->titleLineEdit->setFocus();
    ui->coverImageLabel->clear();
    ui->coverImageLabel->setText( tr( "No Image" ));
    m_cover = QImage();
    cover_page_used = false;
}

void AddItemDialog::closeEvent( QCloseEvent *event )
{
    if( !IsColumnsEmpty() ){
        if( QMessageBox::warning( this, "Close", "You have unsaved data, are you sure you wanna close this?",
                              QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes ){
            event->accept();
            return;
        }
        event->ignore();
        return;
    }
    event->accept();
}

void AddItemDialog::onUploadButtonClicked()
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
        ui->coverImageLabel->clear();
        ui->coverImageLabel->setPixmap( QPixmap::fromImage( image ));
        ui->coverImageLabel->setMaximumSize( QSize( 100, 100 ));
        m_cover = image;
        cover_page_used = true;
    }
}

void AddItemDialog::GenerateReport()
{
    QSqlQuery report_query {};
    report_query.prepare( "INSERT INTO reports( book_title, author_name, stock, date_performed, "
                          "transaction_type, price, total ) VALUES ( :title, :author, "
                          ":stck, :date, :type, :price, :total )");
    report_query.bindValue( ":title", ui->titleLineEdit->text() );
    report_query.bindValue( ":author", ui->authorLineEdit->text() );
    report_query.bindValue( ":stck", ui->stockLineEdit->text().toInt() );
    report_query.bindValue( ":date", QDateTime::currentDateTime() );
    report_query.bindValue( ":type", static_cast<int> ( ReportActionType::ADDITIONS ) );
    report_query.bindValue( ":price", ui->priceLineEdit->text() );
    report_query.bindValue( ":total", ui->priceLineEdit->text().toDouble() *
                            ui->stockLineEdit->text().toInt() );

    if( !report_query.exec() ){
        qDebug() << report_query.lastError();
        QMessageBox::information( this, "Report", "Unable to generate report", QMessageBox::Ok );
        return;
    }
}
