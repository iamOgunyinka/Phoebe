#ifndef REPORT_DIALOG_HPP
#define REPORT_DIALOG_HPP

#include <QDialog>
#include "resources.hpp"

namespace Ui {
class ReportDialog;
}
enum class ReportFormatType {
    PDF = 0,
    CSV,
    INVALID
};

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog( QWidget *parent = 0);
    ~ReportDialog();
private:
    void SetupWindow();
    QSqlQuery GetQuery();
private slots:
    void onFormatChanged( int );
    void onReportChanged( int );
    void onGenerateButtonClicked();
private:
    Ui::ReportDialog *ui;
    ReportFormatType format;
    ReportActionType type;
};

#endif // REPORT_DIALOG_HPP
