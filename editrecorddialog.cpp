#include "editrecorddialog.h"
#include "ui_editrecorddialog.h"

EditRecordDialog::EditRecordDialog(const QStringList &names, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditRecordDialog)
{
    ui->setupUi(this);
    ui->cmbPerson->addItems(names);

    //今日の日付設定
    ui->edtDate->setCalendarPopup(true);
    ui->edtDate->setDate(QDate::currentDate());

    connect (ui->btnOK,
            &QPushButton::clicked,
            this,
            &QDialog::accept);
    connect (ui->btnCancel,
            &QPushButton::clicked,
            this,
            &QDialog::reject);
    connect (ui->cmbPerson,
            &QComboBox::currentTextChanged,
            this,
            &EditRecordDialog::personChanged);
}

EditRecordDialog::~EditRecordDialog()
{
    delete ui;
}

EditRecordDialog::EditMode EditRecordDialog::editMode() const
{
    if (ui->radioDeleteAll->isChecked())
        return DeleteAll;
    if (ui->radioDeleteStart->isChecked())
        return DeleteStart;
    if (ui->radioDeleteEnd->isChecked())
        return DeleteEnd;

    return EditTime;
}

QString EditRecordDialog::personName() const
{
    return ui->cmbPerson->currentText();
}

QDate EditRecordDialog::date() const
{
    return ui->edtDate->date();
}

QTime EditRecordDialog::startTime() const
{
    return ui->edtStart->time();
}

QTime EditRecordDialog::endTime() const
{
    return ui->edtEnd->time();
}