#ifndef ADD_ITEM_DIALOG_HPP
#define ADD_ITEM_DIALOG_HPP

#include <QDialog>
#include <QImage>

namespace Ui {
class InventoryActionDialog;
}

class AddItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddItemDialog( QWidget *parent = 0);
    ~AddItemDialog();

    static QString TABLE_NAME;
protected:
    void closeEvent( QCloseEvent * ) override;
    bool IsColumnsEmpty();
    void ClearEntries();
    void GenerateReport();
private slots:
    void onSaveButtonClicked();
    void onUploadButtonClicked();
private:
    Ui::InventoryActionDialog * ui;
    QImage                      m_cover;
    bool                        cover_page_used;
};

#endif // ADD_ITEM_DIALOG_HPP
