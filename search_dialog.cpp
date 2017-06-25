#include "search_dialog.hpp"
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

SearchDialog::SearchDialog( QString const & title, QWidget *parent ) : QDialog( parent )
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget( new QLabel( "Book title" ), 0, 0 );
    QLineEdit *bookTitleLineEdit = new QLineEdit( title ), *authorNameLineEdit = new QLineEdit;
    QPushButton *searchButton = new QPushButton( "&Search" );

    layout->addWidget( bookTitleLineEdit, 0, 1 );
    layout->addWidget( new QLabel( "Author's name"), 1, 0 );
    layout->addWidget( authorNameLineEdit, 1, 1 );
    layout->addWidget( searchButton, 2, 1 );
    if( !title.isEmpty() ){
        book_title = title;
        authorNameLineEdit->setFocus();
    }

    QObject::connect( bookTitleLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onBookTitleChanged(QString)) );
    QObject::connect( authorNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onAuthorNameChanged(QString)) );
    QObject::connect( searchButton, SIGNAL(clicked(bool)), this, SLOT(accept()) );
    setLayout( layout );
}

void SearchDialog::onAuthorNameChanged( QString const & text )
{
    author_name = text;
}

void SearchDialog::onBookTitleChanged( QString const & text )
{
    book_title = text;
}
