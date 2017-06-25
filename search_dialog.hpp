#ifndef SEARCH_DIALOG_HPP
#define SEARCH_DIALOG_HPP

#include <QDialog>
#include <QWidget>

class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    SearchDialog( QString const &, QWidget *parent = nullptr );
    ~SearchDialog() = default;

    QString const GetBookTitle() const { return book_title; }
    QString const GetAuthorName() const { return author_name; }
private slots:
    void onBookTitleChanged( QString const & );
    void onAuthorNameChanged( QString const & );
private:
    QString     book_title;
    QString     author_name;
};

#endif // SEARCH_DIALOG_HPP
