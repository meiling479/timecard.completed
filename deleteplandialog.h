#ifndef DELETEPLANDIALOG_H
#define DELETEPLANDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class butonBox;
}

class DeletePlanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeletePlanDialog(const QStringList &names, QWidget *parent = nullptr);
    ~DeletePlanDialog();

    QString personName() const;
    QDate fromDate() const;
    QDate toDate() const;
    QString planText() const;

private:
    Ui::butonBox *ui;
    void updatePlanList();
};

#endif // DELETEPLANDIALOG_H
