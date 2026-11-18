#include "plandialog.h"
#include "ui_plandialog.h"
#include <QDate>

PlanDialog::PlanDialog(const QStringList &personList, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PlanDialog)
{
    ui->setupUi(this);
    ui->comboName->addItems(personList);

    ui->lineEditDetail->setVisible(false);
    //今日の日付設定
    ui->dateFrom->setCalendarPopup(true);
    ui->dateFrom->setDate(QDate::currentDate());
    ui->dateTo->setCalendarPopup(true);
    ui->dateTo->setDate(ui->dateFrom->date());

    connect (ui->btnOK,
            &QPushButton::clicked,
            this,
            &QDialog::accept);
    connect (ui->btnCancel,
            &QPushButton::clicked,
            this,
            &QDialog::reject);

    connect (ui->dateFrom,
            &QDateEdit::dateChanged,
            this,
            [this](const QDate &date)
            {
                ui->dateTo->setDate(date);
            });
}

PlanDialog::~PlanDialog()
{
    delete ui;
}

void PlanDialog::on_btnPaid_clicked()
{
    m_selectedPlanType ="有休";
    ui->lineEditDetail->clear();
    ui->lineEditDetail->setVisible(false);
    ui->labelDetailPrompt->setVisible(false);
}

void PlanDialog::on_btnComp_clicked()
{
    m_selectedPlanType ="代休";
    ui->lineEditDetail->clear();
    ui->lineEditDetail->setVisible(false);
    ui->labelDetailPrompt->setVisible(false);
}

void PlanDialog::on_btnTrip_clicked()
{
    m_selectedPlanType ="出張";
    ui->labelDetailPrompt->setText("出張先入力：");
    ui->lineEditDetail->setVisible(true);
    ui->lineEditDetail->setVisible(true);
    ui->lineEditDetail->setPlaceholderText("例：奥尻");
    ui->lineEditDetail->setFocus();
}

void PlanDialog::on_btnOther_clicked()
{
    m_selectedPlanType ="その他";
    ui->labelDetailPrompt->setText("予定内容入力：");
    ui->lineEditDetail->setVisible(true);
    ui->lineEditDetail->setVisible(true);
    ui->lineEditDetail->setPlaceholderText("例：病院");
    ui->lineEditDetail->setFocus();
}

QString PlanDialog::planText() const
{
    QString detail = ui->lineEditDetail->text().trimmed();
    if (m_selectedPlanType == "出張" && !detail.isEmpty())
    {
        return QString("出張(%1)").arg(detail);
    }
    else if (m_selectedPlanType == "その他")
    {
        return detail.isEmpty() ? "その他" : detail;
    }
    return m_selectedPlanType;//有休or代休
}

void PlanDialog::on_buttonBox_acceptted()
{
    accept();//必要に応じて入力チェック
}

QString PlanDialog::personName() const
{
    return ui->comboName->currentText();
}

QDate PlanDialog::fromDate() const
{
    return ui->dateFrom->date();
}
QDate PlanDialog::toDate() const
{
    return ui->dateTo->date();
}



