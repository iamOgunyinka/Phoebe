#ifndef BUY_BOOK_DIALOG_HPP
#define BUY_BOOK_DIALOG_HPP

#include <QDialog>
#include <QList>
#include "resources.hpp"

namespace Ui {
class BuyBookDialog;
}

class BuyBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BuyBookDialog( QList<DatabaseRecordFormat> && list, QWidget *parent = 0);
    ~BuyBookDialog();
private:
    void UpdateNextRecord( int );
private slots:
    void onNextRecord();
    void onPreviousRecord();
    void onItemPurchased();
    void onQuantityChanged( QString );
private:
    Ui::BuyBookDialog *ui;
    int curr_item_index;
    QList<DatabaseRecordFormat> data_list;
    QString old_quantity;
};

#endif // BUY_BOOK_DIALOG_HPP
