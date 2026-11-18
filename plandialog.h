#ifndef PLANDIALOG_H
#define PLANDIALOG_H
#pragma once

#include <QDialog>
#include <QDate>
#include <QStringList>

namespace Ui {
class PlanDialog;
}

class PlanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlanDialog(const QStringList &personList, QWidget *parent = nullptr);
    ~PlanDialog();

    QString planText() const;
    QString personName() const;
    QDate fromDate() const;
    QDate toDate() const;

private slots:
    void on_btnPaid_clicked();
    void on_btnComp_clicked();
    void on_btnTrip_clicked();
    void on_btnOther_clicked();
    void on_buttonBox_acceptted();

private:
    Ui::PlanDialog *ui;
    QString m_selectedPlanType;
};

#endif // PLANDIALOG_H
