#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QStringList personList();

private slots:
    QString timecardPath() const;
    QString planPath() const;
    QString normalize(const QString &s) const;
    void refreshView();
    void updateClock();
    void updateNameButtons();
    void initMonthTable(int year, int month);
    void checkMonthChange();
    void loadCsvToTable(const QString &name);
    void writeRecord(const QString &type);
    void savePlan(const QDate &from, const QDate &to, const QString &name, const QString &plan);
    void on_btnAddPlan_clicked();
    void deletePlan(const QDate &from, const QDate &to, const QString &name, const QString &plan);
    void on_btnDeletePlan_clicked();
    void on_btnEditRecord_clicked();
    void loadPlanToTable(const QString &name);
    bool hasTodayPlan(const QString &name);
    void loadTodayPlans();
    void updateRecord(const QString &name, const QDate &date, const QTime &startTime, const QTime &endTime);
    void deleteRecord(const QString &name, const QDate &date, const QString &type);
    void on_btnPrevMonth_clicked();
    void on_btnNextMonth_clicked();
    QStringList getPlans(const QDate &date, const QString &name);
    void exportMonthlyReport();

private:
    Ui::MainWindow *ui;

    QList<QPushButton*> nameButtons();
    QPushButton *selectedButton = nullptr;

    QString getTodayStatus(const QString &name);
    QString currentName;
    QString leaveConfirmName;
    int currentYear;
    int currentMonth;
    QDate nthMonday(int year, int month, int nth) const;
    QDate springEquinox(int year) const;
    QDate autumnEquinox(int year) const;
    bool isHoliday(const QDate &date) const;

};
#endif // MAINWINDOW_H
