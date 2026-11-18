#ifndef EDITRECORDDIALOG_H
#define EDITRECORDDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QComboBox>
#include <QTimeEdit>

namespace Ui {
class EditRecordDialog;
}

class EditRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRecordDialog(const QStringList &names, QWidget *parent = nullptr);
    ~EditRecordDialog();

    QString personName() const;
    QDate date() const;
    QTime startTime() const;
    QTime endTime() const;
    bool deleteRecord() const;
    enum EditMode
    {
        EditTime, DeleteAll, DeleteStart, DeleteEnd
    };
    EditMode editMode() const;

private:
    Ui::EditRecordDialog *ui;

    QComboBox *cmbPerson;
    QDateEdit *edtDate;
    QTimeEdit *edtStart;
    QTimeEdit *edtEnd;

signals:
    void personChanged(const QString &name);

};

#endif // EDITRECORDDIALOG_H
