#ifndef VIEW_INVENTORY_DIALOG_HPP
#define VIEW_INVENTORY_DIALOG_HPP

#include <QDialog>
#include <QList>
#include <QDateTime>
#include "resources.hpp"

namespace Ui {
class ViewInventoryDialog;
}


enum class ActionType
{
    View,
    Delete,
    Update
};

class QSqlQuery;

class ViewInventoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ViewInventoryDialog( ActionType action, QWidget *parent = nullptr );
    ~ViewInventoryDialog();
    void CheckDatabaseRecord();

    void SetDataList( QList<DatabaseRecordFormat> && list );

private slots:
    void onNextRecord();
    void onPreviousRecord();
    void onDeleteButtonClicked();
    void onUpdateButtonClicked();
    void onUploadButtonClicked();
private:
    void UpdateNextRecord( int );
    void SetupWindowForDelete();
    void SetupWindowForUpdate();
    void GenerateReport( ActionType );
private:
    Ui::ViewInventoryDialog     *ui;
    QList<DatabaseRecordFormat> data_list; // a linked-list of database data
    int                         curr_record_index;
    QImage                      m_image;
};

#endif // VIEW_INVENTORY_DIALOG_HPP
