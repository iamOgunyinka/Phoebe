#include "report_dialog.hpp"
#include "ui_report_dialog.h"

#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QPrinter>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QStringList>
#include <QTextDocument>
#include <QTextStream>

ReportDialog::ReportDialog(QWidget *parent) :
    QDialog(parent),
    ui( new Ui::ReportDialog ), type{ ReportActionType::ALL }, format{ ReportFormatType::PDF }
{
    ui->setupUi(this);

    SetupWindow();
    setMaximumSize( 320, 270 );
    setWindowTitle( "Generate report" );

    QObject::connect( ui->formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatChanged(int)) );
    QObject::connect( ui->reportTypeComboBox, SIGNAL(currentIndexChanged(int)), this,
                      SLOT(onReportChanged(int)) );
    QObject::connect( ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(onGenerateButtonClicked()) );
}

ReportDialog::~ReportDialog()
{
    delete ui;
}

void ReportDialog::SetupWindow()
{
    QStringList report_type_list, format_type_list;
    report_type_list << "All" << "Sales" << "Updates" << "Deletions" << "Addtions";
    ui->reportTypeComboBox->addItems( report_type_list );

    format_type_list << "PDF" << "CSV";
    ui->formatComboBox->addItems( format_type_list );
    ui->toDateTimeEdit->setDateTime( QDateTime::currentDateTime() );
}

void ReportDialog::onFormatChanged( int new_format )
{
    if( static_cast<int>( format ) != new_format ){
        format = static_cast<ReportFormatType>( new_format );
    }
}
void ReportDialog::onReportChanged( int new_report_type )
{
    if( static_cast<int>( type ) != new_report_type ){
        type = static_cast<ReportActionType>( new_report_type );
    }
}

QSqlQuery ReportDialog::GetQuery()
{
    QString to_date = GetDateTime( ui->toDateTimeEdit->dateTime() ),
            from_date = GetDateTime( ui->fromDateTimeEdit->dateTime() );

    bool const is_generating_all = ( type == ReportActionType::ALL );
    QString query_string = tr( "SELECT * FROM reports WHERE "
                               "( date_performed >= :from && date_performed <= :to ) %1" )
            .arg( is_generating_all? "" : "&& transaction_type = :type ");

    QSqlQuery query {};
    query.prepare( query_string );
    query.bindValue(":from", from_date );
    query.bindValue( ":to", to_date );
    if( !is_generating_all ){
        query.bindValue( ":type", static_cast<int>( type ) );
    }

    return query;
}

void ReportDialog::onGenerateButtonClicked()
{
    bool const is_csv = ( format == ReportFormatType::CSV );

    QSqlQuery select_all_query { GetQuery() };

    if( !select_all_query.exec() ){
        qDebug() << select_all_query.lastError();
        qDebug() << "Executed query" << select_all_query.executedQuery();
        QMessageBox::critical( this, "Report", "Unable to generate report from the database" );
        return;
    }
    QList<ReportFormat> data_list {};

    FillReportFromQuery( data_list, select_all_query );
    if( data_list.isEmpty() ){
        QMessageBox::information( this, "Report", "There's nothing to report at the moment");
        return;
    }

    QString const filename = QFileDialog::getSaveFileName( this, tr("Save file"), "",
                                                           tr( is_csv ? "CSV (*.csv)" : "PDF(*.pdf)" ) );

    if( filename.isNull() ) return;

    if( is_csv ){
        QFile file{ filename };
        if( !file.open( QIODevice::WriteOnly ) ){
            QMessageBox::critical( this, "Save report", "Error saving report file, try again later",
                                   QMessageBox::Ok );
            return;
        }
        QTextStream file_stream(&file);
        file_stream << "book_title, author_name, stock, date_perfomed, transaction_type\r\n";
        for( auto const & data: data_list ){
            file_stream << data.book_title << ", " << data.author_name << ", " << data.quantity << ", "
                        << data.date_time_added.toString() << ", " << Stringify( data.detail ) << "\r\n";
        }
        file.close();
    } else { // it's a PDF
        QString pdf_string(
                    tr( "<div align=\"center\"><big>Bookshop Report( %1 )</big><br><br><br>"
                        "<table border = \"1\"><tr>"
                        "<th>Title</th>"
                        "<th>Author</th>"
                        "<th>Quantity</th>"
                        "<th>Price</th>"
                        "<th>Total</th>"
                        "<th>Transaction</th>"
                        "<th>Date</th></tr>" )
                    .arg( type == ReportActionType::ALL ? "All transactions" : Stringify( type )));
        for( auto const & data: data_list ){
            pdf_string += ( "<tr><td>" + data.book_title + "</td><td>" + data.author_name +
                            "</td><td>" + QString::number( data.quantity ) + "</td>" +
                            "<td>" + QString::number( data.price ) + "</td>" +
                            "<td>" + QString::number( data.total ) + "</td><td>" +
                            Stringify( data.detail ) + "</td><td>" + data.date_time_added.toString() +
                            "</td></tr>");
        }

        pdf_string += "</table></div>";
        QPrinter printer( QPrinter::PrinterResolution );
        printer.setOutputFormat( QPrinter::PdfFormat );
        printer.setPaperSize( QPrinter::A4 );
        printer.setOutputFileName( filename );

        QTextDocument htmlDoc {};
        htmlDoc.setHtml( pdf_string );
        htmlDoc.setPageSize( printer.pageRect().size() );
        htmlDoc.print( &printer );
    }
    QMessageBox::information( this, "Report", tr( "Report has been generated successfully into %1")
                              .arg( filename ));
}
