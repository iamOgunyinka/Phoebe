#include <QGridLayout>
#include <QWidget>
#include <QMessageBox>
#include <QDebug>
#include <fstream>

#include "app_main_window.hpp"
#include "login_dialog.hpp"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog( parent )
{
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode( QLineEdit::Password );
    passwordEdit->setToolTip( tr( "Enter password to unlock shop"));

    loginButton = new QPushButton( tr( "&Login" ));
    cancelButton = new QPushButton( tr("&Cancel"));
    passwordLabel = new QLabel( tr( "Enter password"));

    QHBoxLayout *editLayout = new QHBoxLayout();
    editLayout->addWidget( passwordLabel );
    editLayout->addWidget( passwordEdit );

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget( loginButton );
    buttonsLayout->addWidget( cancelButton );

    QGridLayout *windowLayout = new QGridLayout();
    windowLayout->addLayout( editLayout, 0, 0 );
    windowLayout->addItem( new QSpacerItem( 10, 40, QSizePolicy::Fixed ), 1, 0 );
    windowLayout->addLayout( buttonsLayout, 2, 0 );

    setLayout( windowLayout );
    setFixedHeight( 100 );
    setFixedWidth( 220 );

    QObject::connect( cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    QObject::connect( loginButton, SIGNAL(clicked(bool)), this, SLOT(loginClicked()) );

    this->setWindowIcon( QIcon( ":/new/icons/icons/logo.png") );
}

LoginDialog::~LoginDialog()
{

}

void LoginDialog::loginClicked()
{
    passwordText = passwordEdit->text();
    auto const filename = "data.dat";
    std::fstream ifile( filename, std::ios::binary | std::ios::in );

    if( !ifile ){
        // we do not yet have a password file, create one
        std::ofstream ofile( filename, std::ios::binary | std::ios::out );
        if( !ofile ){
            // we must be having an issue here.
            QMessageBox::critical( this, tr( "Error" ), tr( "Unable to create password file" ),
                                   QMessageBox::Ok );
            qDebug() << tr( "Unable to create a password file" );
            std::exit( -1 );
        }

        ofile << this->encodeDecode( std::string( "scope" ) );
        ofile.close();

        ifile.open( filename, std::ios::binary | std::ios::in );
        if( !ifile ){
            QMessageBox::critical( this, tr("Error"), tr("Unable to open password file"), QMessageBox::Ok );
            qDebug() << tr( "unable to read password file" );
            std::exit( -1 );
        }
    }
    // if we are here, then we've been able to open the password file
    std::string encodedPassword {};
    std::getline( ifile, encodedPassword );
    ifile.close();

    if( this->encodeDecode( encodedPassword ) != passwordText.toStdString() ){
        QMessageBox::warning( this, tr("Login"), tr("Login failed"), QMessageBox::Ok );
        std::exit( -1 );
    }

    QMessageBox::information( this, tr( "Login" ), tr( "Logged in successfully." ), QMessageBox::Ok );
    onLoginSuccessful();
}

std::string LoginDialog::encodeDecode( std::string const & text )
{
    std::string newText { text };
    for( std::string::size_type i = 0; i != newText.size(); ++i ){
        newText[i] = ~newText[i];
    }
    return newText;
}

void LoginDialog::onLoginSuccessful()
{
    this->hide();

    AppMainWindow *main_dialog = new AppMainWindow( this );
    QObject::connect( main_dialog, SIGNAL(destroyed(QObject*)), this, SLOT(close()) );

    main_dialog->show();
}
