#ifndef RESOURCES_HPP
#define RESOURCES_HPP

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QList>
#include <QVariant>

enum class ReportActionType {
    ALL = 0,
    SALES,
    UPDATES,
    DELETIONS,
    ADDITIONS
};

struct DatabaseRecordFormat
{
    unsigned int    serial_number; // UNIQUE, only used internally to recognize individual records
    unsigned int    quantity;
    double          price;
    QString         book_title;
    QString         author_name;
    QString         publisher;
    QDateTime       date_time_added;
    QString         location; // where in the "inventory" it is physically located.
    QByteArray      book_cover; // could be BLOB data or NULL
};

struct ReportFormat
{
    unsigned int        serial_number;
    int                 quantity;
    double              price;
    double              total;
    QString             book_title;
    QString             author_name;
    QDateTime           date_time_added;
    ReportActionType    detail;
};

static void FillRecordFromQuery( QList<DatabaseRecordFormat> &list, QSqlQuery &query)
{
    QSqlRecord record {};
    while( query.next() ){
        record = query.record();
        bool const has_cover = record.contains( "book_cover" );
        DatabaseRecordFormat data =
        {
            record.value( "serial_number" ).toInt(), // serial number
            record.value( "stock" ).toInt(), // stock number
            record.value( "price" ).toDouble(),
            record.value( "book_title" ).toString(), // book's title
            record.value( "author_name" ).toString(), // author's name
            record.value( "publisher" ).toString(), // publisher
            record.value( "date_time").toDateTime(), // date time
            record.value( "location" ).toString(), // location
            has_cover == true ? record.value( "book_cover" ).toByteArray() : QByteArray() // cover page
        };
        list.append( data );
    }
}
static void FillReportFromQuery( QList<ReportFormat> &data_list, QSqlQuery &query )
{
    QSqlRecord record {};
    while( query.next() ){
        record = query.record();
        ReportFormat data = {
            record.value( "serial_number" ).toInt(), // serial number
            record.value( "stock" ).toInt(), // stock number
            record.value( "price" ).toDouble(),
            record.value( "total" ).toDouble(),
            record.value( "book_title" ).toString(), // book's title
            record.value( "author_name" ).toString(), // author's name
            record.value( "date_performed").toDateTime(), // date time
            static_cast<ReportActionType>( record.value( "transaction_type" ).toInt() ), // location
        };
        data_list.append( data );
    }
}

static QString GetDateTime( QDateTime const & date_time )
{
    QString date_string = date_time.date().toString( "yyyy-MM-dd"),
            time_string = date_time.time().toString("HH-mm-ss");
    return date_string + " " + time_string;
}

static QString Stringify( ReportActionType type )
{
    switch( type ){
    case ReportActionType::ADDITIONS:
        return QString( "Addition" );
    case ReportActionType::DELETIONS:
        return QString( "Deletion" );
    case ReportActionType::SALES:
        return QString( "Sales" );
    case ReportActionType::UPDATES:
        return QString( "Update" );
    case ReportActionType::ALL:
    default:
        return QString( "Unknown" );
    }
}

#endif // RESOURCES_HPP
