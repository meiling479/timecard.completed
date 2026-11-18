#include "deleteplandialog.h"
#include "ui_deleteplandialog.h"

#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDate>
#include <QStringList>

DeletePlanDialog::DeletePlanDialog(const QStringList &names, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::butonBox)
{
    ui->setupUi(this);
    ui->comboBoxName->addItems(names);

    //今日の日付設定
    ui->dateFrom->setCalendarPopup(true);
    ui->dateFrom->setDate(QDate::currentDate());
    ui->dateTo->setCalendarPopup(true);
    ui->dateTo->setDate(ui->dateFrom->date());

    connect (ui->deleteButton,
            &QPushButton::clicked,
            this,
            &QDialog::accept);

    connect (ui->comboBoxName,
            &QComboBox::currentTextChanged,
            this,
            &DeletePlanDialog::updatePlanList);

    connect (ui->dateFrom,
            &QDateEdit::dateChanged,
            this,
            [this](const QDate &date)
                {
                ui->dateTo->setDate(date);
            });

    connect (ui->dateTo,
            &QDateEdit::dateChanged,
            this,
            &DeletePlanDialog::updatePlanList);
}

DeletePlanDialog::~DeletePlanDialog()
{
    delete ui;
}

QString DeletePlanDialog::personName() const
{
    return ui->comboBoxName->currentText();
}

QDate DeletePlanDialog::fromDate() const
{
    return ui->dateFrom->date();
}
QDate DeletePlanDialog::toDate() const
{
    return ui->dateTo->date();
}

QString DeletePlanDialog::planText() const
{
    if (!ui->planList->currentItem())
        return "";
    return ui->planList->currentItem()->text();
}

void DeletePlanDialog::updatePlanList()
{
    ui->planList->clear();

    QString path  = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                   + "/plan.csv";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 3)
            continue;

        QDate date = QDate::fromString(cols[0], "yyyy/MM/dd");
        QString name = cols[1].trimmed();
        QString plan = cols[2].trimmed();
        if (date == ui->dateFrom->date() && name == ui->comboBoxName->currentText())
        {
            ui->planList->addItem(plan);
        }
    }
    file.close();
}