#include "Mainwindow.h"
#include "ui_Mainwindow.h"
#include "plandialog.h"
#include "deleteplandialog.h"
#include "editRecordDialog.h"

#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //起動時初期化
    currentYear = QDate::currentDate().year();
    currentMonth = QDate::currentDate().month();

    initMonthTable(currentYear, currentMonth);
    loadTodayPlans();

    connect (ui->btnExportReport,
            &QPushButton::clicked,
            this,
            &MainWindow::exportMonthlyReport);

    //名前ボタン接続
    for (auto b : nameButtons()){
        if (b->text().isEmpty()) continue;

        connect (b, &QPushButton::clicked, this, [=](){
            selectedButton = b;
            QString name = b->text().trimmed();
            if (currentName != name)
            {
                leaveConfirmName.clear();
            }
            currentName = name;

            ui->labelName->setText(QString("表示中: %1").arg(name));

            //テーブル表示、csv読み込み
            initMonthTable(currentYear, currentMonth);
            loadCsvToTable(name);

            //予定有→出退処理しない
            if (hasTodayPlan(name))
            {
                return;
            }

            //状態判断
            QString status = getTodayStatus(name);

            //出退処理
            if (status.isEmpty())
            {
                writeRecord("出勤");
                b->setStyleSheet("background-color: lightblue");
                loadCsvToTable(name);
                return;
            }
            if (status == "出勤済み")
            {
                //１回目＝テーブル表示
                if (leaveConfirmName != name)
                {
                    leaveConfirmName = name;
                    ui->labelName->setText(QString("表示中: %1 （２回目で退勤)").arg(name));
                    return;
                }
                //2回目=退勤処理
                leaveConfirmName.clear();
                writeRecord("退勤");
                b->setStyleSheet("");

            //反映
            initMonthTable(currentYear, currentMonth);
            loadCsvToTable(name);
            loadTodayPlans();
            updateNameButtons();
            return;
            }
        });
    }

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
    connect(timer, &QTimer::timeout, this, &MainWindow::checkMonthChange);
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//ー共通関数ー
QString MainWindow::timecardPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
           + "/timecard.csv";
}

QString MainWindow::planPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
           + "/plan.csv";
}

QString MainWindow::normalize(const QString &s) const
{
    return QString(s).trimmed()
                     .replace(" ", "")
                     .replace("　", "")
                     .remove('\r')
                     .remove('\n')
                     .remove('\t');
}

void MainWindow::refreshView()
{
    initMonthTable(currentYear, currentMonth);
    if (!currentName.isEmpty())
    {
        loadCsvToTable(currentName);
    }
}

//ー時間、祝日ー
void MainWindow::updateClock()//レベルで時間表示
{
    ui->labelTime->setText(
        QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss")
    );
}

QDate MainWindow::nthMonday(int year, int month, int nth) const//ハッピーマンデー計算
{
    QDate d(year, month, 1);
    while (d.dayOfWeek() != Qt::Monday)
        d = d.addDays(1);
    return d.addDays((nth - 1) * 7);
}

QDate MainWindow::springEquinox(int year) const//春分の日計算
{
    int day = static_cast<int>(20.8431+0.242194*(year-1980)-(year-1980)/4);
    return QDate(year, 3, day);
}

QDate MainWindow::autumnEquinox(int year) const//秋分の日計算
{
    int day = static_cast<int>(23.2488+0.242194*(year-1980)-(year-1980)/4);
    return QDate(year, 9, day);
}

bool MainWindow::isHoliday(const QDate &date) const//祝日判定
{
    int year = date.year();
    QList<QDate> holidays;

    //固定祝日
    holidays << QDate(year, 1, 1);//元日
    holidays << QDate(year, 2, 11);//建国記念日
    holidays << QDate(year, 2, 23);//天皇誕生日
    holidays << QDate(year, 4, 29);//昭和の日
    holidays << QDate(year, 5, 3);//憲法記念日
    holidays << QDate(year, 5, 4);//みどりの日
    holidays << QDate(year, 5, 5);//こどもの日
    holidays << QDate(year, 8, 11);//山の日
    holidays << QDate(year, 11, 3);//文化の日
    holidays << QDate(year, 11, 23);//敬老感謝の日

    //ハッピーマンデー
    holidays << nthMonday(year, 1, 2);//成人の日
    holidays << nthMonday(year, 7, 3);//海の日
    holidays << nthMonday(year, 9, 3);//敬老の日
    holidays << nthMonday(year, 10, 2);//スポーツの日

    //春分、秋分
    holidays << springEquinox(year);
    holidays << autumnEquinox(year);

    //国民の休日
    QDate keirou = nthMonday(year, 9, 3);
    QDate shuubun = autumnEquinox(year);

    if (keirou.daysTo(shuubun) == 2)
        holidays << keirou.addDays(1);
    return holidays.contains(date);
}

//ー月移動ボタンー
void MainWindow::on_btnPrevMonth_clicked()//前月ボタン
{
    currentMonth--;
    if (currentMonth < 1)
    {
        currentMonth = 12;
        currentYear--;
    }
    refreshView();
}

void MainWindow::on_btnNextMonth_clicked()//次月ボタン
{
    currentMonth++;
    if (currentMonth > 12)
    {
        currentMonth = 1;
        currentYear++;
    }
    refreshView();
}

//ーボタン、名前リストー
QList<QPushButton*> MainWindow::nameButtons()
{
    QList<QPushButton*> list;
    for (auto *b : this->findChildren<QPushButton*>()){
             if (b->objectName().startsWith("name") && !b->text().isEmpty()){
                 list.append(b);
             }
         }
        return list;
}

QStringList MainWindow::personList()
{
    QStringList list;
    for (auto b : nameButtons())
    {
        list << b->text().trimmed();
    }
    return list;
}

void MainWindow::updateNameButtons()
{
    for (auto b : nameButtons())
    {
        QString name = b->text().trimmed();

        if (getTodayStatus(name) == "出勤済み")
            b->setStyleSheet("background-color: lightblue");
        else b->setStyleSheet("");
    }
}

//ー打刻ー
void MainWindow::writeRecord(const QString &type)
{
    if (!selectedButton) return;

    QString name = selectedButton->text().trimmed();
    QDateTime now = QDateTime::currentDateTime();

    QFile file(timecardPath());
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::System);
    //csv形式
    out << now.toString("yyyy/MM/dd") << ","
        << now.toString("HH:mm:ss") << ","
        << name << ","
        << type << "\n";

    file.close();
}

//-csv月報方式-
void MainWindow::exportMonthlyReport()
{
    QSet<QString> names;
    QSet<QDate> allDates;
    QMap<QDate, QMap<QString, QString>> startTimes;
    QMap<QDate, QMap<QString, QString>> endTimes;
    QMap<QDate,QMap<QString, QString>> plans;

    QString inputPath = timecardPath();
    QString outputPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                         + "/monthly_report.csv";

    QFile file(inputPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);

    QFile planFile(planPath());
    if (planFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream pin(&planFile);
        pin.setEncoding(QStringConverter::System);
        while (!pin.atEnd())
        {
            QStringList cols = pin.readLine().split(",");
            if (cols.size() < 3) continue;

            QDate d = QDate::fromString(cols[0].trimmed(), "yyyy/MM/dd");
            QString person = cols[1].trimmed();
            QString plan = cols[2].trimmed();
            plans[d][person] = plan;
        }
        planFile.close();
    }

    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList cols = line.split(",");
        if (cols.size() < 4) continue;

        QDate date =QDate::fromString(cols[0].trimmed(), "yyyy/MM/dd");
        QString time = cols[1].trimmed();
        QString name = cols[2].trimmed();
        QString type = cols[3].trimmed();
        if (!date.isValid()) continue;

        names.insert(name);
        allDates.insert(date);
        if (type == "出勤")
        {
            startTimes[date][name] = time;
        }
        else if (type == "退勤")
        {
            endTimes[date][name] = time;
        }
    }
    file.close();

    QFile outfile(outputPath);
    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        return;
    }
    QTextStream out(&outfile);
    out.setEncoding(QStringConverter::System);

    QStringList nameList = personList();

    out << "日付,曜日";

    for (const QString &name : std::as_const(nameList))
    {
        out << "," << name;
    }
    out << "\n";

    QList<QDate> dates = allDates.values();
    std::sort(dates.begin(), dates.end());

    for(const QDate &date : std::as_const(dates))
    {
        QString youbi = QLocale(QLocale::Japanese).toString(date, "ddd");

        out << date.toString("yyyy/MM/dd") << "," << youbi;

        for (const QString &name : std::as_const(nameList))
        {
            QString start = startTimes[date].value(name);
            QString end = endTimes[date].value(name);
            QString plan = plans[date].value(name);
            QString work;
            if (!start.isEmpty() && !end.isEmpty())
            {
                work =start.left(5) + "-" + end.left(5);
            }
            else if (!plan.isEmpty())
            {
                work = plan;
            }
            else if (!start.isEmpty())
            {
                work = start.left(5);
            }
            else if (!end.isEmpty())
            {
                work = "-" + end.left(5);
            }
            out << "," << work;
        }
        out << "\n";
    }
    outfile.close();
    QDesktopServices::openUrl(QUrl::fromLocalFile(outputPath));
}

void MainWindow::loadCsvToTable(const QString &name)
{
    QFile file(timecardPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);

    QString n = normalize(name);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");

        if (cols.size() < 4) continue;

        QString date = cols[0].trimmed();
        QString time = cols[1].trimmed();
        QString person = cols[2].trimmed();
        QString type = cols[3].trimmed();

        if (normalize(person) != n) continue;

        QDate d = QDate::fromString(date, "yyyy/MM/dd");
        int rows = ui->TimeCardTable->rowCount();

        for (int r = 0; r < rows; r++){
            int month = ui->TimeCardTable->item(r, 0)->text().toInt();
            int day   = ui->TimeCardTable->item(r, 1)->text().toInt();

            if (month == d.month() && day == d.day())
            {
                int targetCol = -1;

                if (type == "出勤") targetCol = 3;
                else if (type == "退勤") targetCol = 4;

                if (targetCol >= 0)
                {
                    QTableWidgetItem *item = ui->TimeCardTable->item(r, targetCol);
                    if (!item)
                    {
                        item = new QTableWidgetItem;
                        ui->TimeCardTable->setItem(r, targetCol, item);
                    }
                    item->setText(time.left(5));
                    item->setTextAlignment(Qt::AlignCenter);
                }
                break;
            }
        }
    }
    file.close();
    loadPlanToTable(name);
}

QString MainWindow::getTodayStatus(const QString &name)
{
    QFile file(timecardPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return"";

    QTextStream in(&file);
   in.setEncoding(QStringConverter::System);

    QDate today = QDate::currentDate();
    QString n = normalize(name);
    QString lastType;   // 最後に記録された種別（出勤 or 退勤）

    while (!in.atEnd()){
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 4) continue;

        QDate d = QDate::fromString(cols[0], "yyyy/MM/dd");
        if (!d.isValid()) continue;

        if (d == today && normalize(cols[2]) == n){
            lastType = cols[3].trimmed();
        }
    }
    file.close();

    if (lastType == "退勤") return "退勤済み";
    if (lastType == "出勤") return "出勤済み";
    return "";
}

//-タイムカードテーブル-
void MainWindow::initMonthTable(int year, int month)
{
    ui->labelMonth->setText(QString("%1年%2月").arg(year) .arg(month));

    for (auto b : nameButtons()){
        QString name = b->text().trimmed();

        if (getTodayStatus(name) == "出勤済み"){
            b->setStyleSheet("background-color: lightblue");
        }
        else
        {
            b->setStyleSheet("");
        }
    }

    ui->TimeCardTable->clear();
    ui->TimeCardTable->setRowCount(0);
    ui->TimeCardTable->setColumnCount(6);

    QStringList headers;
    headers << "月" << "日" << "曜日" << "出勤" << "退勤" << "予定";
    ui->TimeCardTable->setHorizontalHeaderLabels(headers);

    QDate firstDay(year, month, 1);
    int days = firstDay.daysInMonth();

    for (int d = 1; d <= days; d++){
        int row = ui->TimeCardTable->rowCount();
        ui->TimeCardTable->insertRow(row);

        QDate date(year, month, d);
        QString youbi = QLocale(QLocale::Japanese).toString(date, "ddd");

        auto set =[&](int col, const QString &text){
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);

            //祝日
            if (isHoliday(date))
            {
                item->setForeground(QBrush(Qt::red));
            }
            //土曜日
            else if (date.dayOfWeek() == 6){
                item->setForeground(QBrush(Qt::blue));
            }
            //日曜日
            else if (date.dayOfWeek() == 7){
                item->setForeground(QBrush(Qt::red));
            }
            ui->TimeCardTable->setItem(row, col, item);
        };

        set(0, QString::number(month));//月
        set(1, QString::number(d));//日
        set(2, youbi);//曜日
        set(3, "");//出勤
        set(4, "");//退勤
        set(5, "");//予定
    }
    //列幅固定
    ui->TimeCardTable->setColumnWidth(0, 30);//月
    ui->TimeCardTable->setColumnWidth(1, 30);//日
    ui->TimeCardTable->setColumnWidth(2, 40);//曜日
    ui->TimeCardTable->setColumnWidth(3, 80);//出勤
    ui->TimeCardTable->setColumnWidth(4, 80);//退勤
    ui->TimeCardTable->setColumnWidth(5, 180);//予定
    //行番号を消す
    ui->TimeCardTable->verticalHeader()->setVisible(false);
}

void MainWindow::checkMonthChange()
{
    static int lastMonth = QDate::currentDate().month();
    QDate today = QDate::currentDate();

    if (today.month() != lastMonth){
        lastMonth = today.month();
        currentYear = today.year();
        currentMonth = today.month();
        refreshView();
    }
}

//ー予定ー
void MainWindow::savePlan(const QDate &from, const QDate &to, const QString &name, const QString &plan)
{
    QFile file(planPath());
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::System);

    for (QDate d = from; d <= to; d = d.addDays(1))
    {
        out << d.toString("yyyy/MM/dd") << ","
        << name << ","
        << plan << "\n";
    }
    file.close();
}

//-予定追加ボタン-
void MainWindow::on_btnAddPlan_clicked()
{
    PlanDialog dlg(personList(), this);
    if (dlg.exec() == QDialog::Accepted)
    {
        currentName = dlg.personName();
        ui->labelName->setText(QString("表示中: %1").arg(currentName));

        savePlan(dlg.fromDate(), dlg.toDate(), dlg.personName(), dlg.planText());
        refreshView();
        loadTodayPlans();
        updateNameButtons();
    }
}

QStringList MainWindow::getPlans(const QDate &date, const QString &name)
{
    QStringList plans;
    QFile file(planPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return plans;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 3) continue;

        QDate planDate = QDate::fromString(cols[0].trimmed(), "yyyy/MM/dd");
        QString person = normalize(cols[1]);
        if (planDate == date && person == normalize(name))
        {
            plans << cols[2];
        }
    }
    file.close();
    return plans;
}

//-予定削除-
void MainWindow::deletePlan(const QDate &from, const QDate &to, const QString &name, const QString &plan)
{
    QFile file(planPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QStringList lines;
    QTextStream in(&file);
   in.setEncoding(QStringConverter::System);

    QString targetName = normalize(name);
    QString targetPlan = normalize(plan);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 3)
        {
            lines << line;
            continue;
        }

        QDate date = QDate::fromString(cols[0].trimmed(), "yyyy/MM/dd");
        QString person = normalize(cols[1]);
        QString planText = normalize(cols[2]);

        if (date >= from && date <= to &&
            person == targetName &&
            planText == targetPlan)
        {
            continue;//削除対象
        }
        lines << line;
    }
    file.close();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::System);
    for (const QString &line : lines)
    {
        out <<line << "\n";
    }
    file.close();
}

//-予定削除ボタン-
void MainWindow::on_btnDeletePlan_clicked()
{
    DeletePlanDialog dlg(personList(), this);
    if (dlg.exec() != QDialog::Accepted) return;

    deletePlan(dlg.fromDate(), dlg.toDate(), dlg.personName(), dlg.planText());
    loadTodayPlans();
    updateNameButtons();
    refreshView();
}

//-打刻修正-
void MainWindow::updateRecord(const QString &name, const QDate &date, const QTime &startTime, const QTime &endTime)
{
    QFile file(timecardPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QStringList lines;
    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols =line.split(",");
        if (cols.size() < 4)
        {
            lines << line;
            continue;
        }

        QDate d = QDate::fromString(cols[0].trimmed(), "yyyy/MM/dd");
        QString person = normalize(cols[2]);
        bool samePerson = person == normalize(name);
        bool sameDate = d == date;
        if (samePerson && sameDate)
        {
            continue;//該当日既存打刻を一旦除外
        }
        lines << line;
    }
    file.close();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::System);
    for (const QString &line : std::as_const(lines))
    {
        out << line << "\n";
    }
        out << date.toString("yyyy/MM/dd") << ","
            << startTime.toString("HH:mm:ss") << ","
            << name << ",出勤\n";

        out << date.toString("yyyy/MM/dd") << ","
            << endTime.toString("HH:mm:ss") << ","
            << name << ",退勤\n";

    file.close();
}

//-打刻削除ボタン-
void MainWindow::on_btnEditRecord_clicked()
{
    EditRecordDialog dlg(personList(), this);

    connect (&dlg,
            &EditRecordDialog::personChanged,
            this,
            [=](const QString &name)
            {
                currentName = name;
                ui->labelName->setText(QString("表示中: %1").arg(name));
                refreshView();
            });
    if (dlg.exec() != QDialog::Accepted) return;
    currentName = dlg.personName();
    switch (dlg.editMode())
    {
    case EditRecordDialog::EditTime:updateRecord(dlg.personName(), dlg.date(), dlg.startTime(), dlg.endTime());
        break;
    case EditRecordDialog::DeleteAll:deleteRecord(dlg.personName(), dlg.date(), "");
        break;
    case EditRecordDialog::DeleteStart:deleteRecord(dlg.personName(), dlg.date(), "出勤");
        break;
    case EditRecordDialog::DeleteEnd:deleteRecord(dlg.personName(), dlg.date(), "退勤");
        break;
    }
    refreshView();
}

void MainWindow::deleteRecord(const QString &name, const QDate &date, const QString &type)
{
    QFile file(timecardPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QStringList lines;
    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 4)
        {
            lines << line;
            continue;
        }

        QDate d = QDate::fromString(cols[0].trimmed(), "yyyy/MM/dd");
        QString person = normalize(cols[2]);
        QString recordType = cols[3].trimmed();

        bool samePerson = person == normalize(name);
        bool sameDate = d == date;

        //DeleteAll
        if (!samePerson || !sameDate)
        {
            lines << line;
            continue;
        }
        //出勤or退勤だけ削除
        if (type.isEmpty())
        {
            continue;
        }
        if (recordType == type)
        {
            continue;
        }
        lines << line;
    }
    file.close();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
    return;
    }

    QTextStream out(&file);
   out.setEncoding(QStringConverter::System);
    for (const QString &line : std::as_const(lines))
    {
        out << line << "\n";
    }
    file.close();
}

void MainWindow::loadPlanToTable(const QString &name)
{
    QFile file(planPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);

    QString targetName = normalize(name);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 3) continue;

        QString dateStr = cols[0].trimmed();
        QDate date = QDate::fromString(dateStr, "yyyy/MM/dd");
        if (!date.isValid()) continue;

        QString person = normalize(cols[1]);
        QString plan = cols[2].trimmed();
        //名前が一致するか
        if (person != targetName) continue;

        for (int r = 0; r < ui->TimeCardTable->rowCount(); r++)
        {
            int month = ui->TimeCardTable->item(r, 0)->text().toInt();
            int day = ui->TimeCardTable->item(r, 1)->text().toInt();

            //年月日すべて一致する場合予定設定
            if (date.year() == currentYear &&
                date.month() == currentMonth &&
                month == date.month() &&
                day == date.day())
            {
                QTableWidgetItem *item = new QTableWidgetItem(plan);
                item->setTextAlignment(Qt::AlignCenter);
                ui->TimeCardTable->setItem(r, 5, item);
                break;
            }
        }
    }
    file.close();
}

bool MainWindow::hasTodayPlan(const QString &name)
{
    QFile file(planPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
   in.setEncoding(QStringConverter::System);

    QString today = QDate::currentDate().toString("yyyy/MM/dd");
    QString targetName = normalize(name);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");
        if (cols.size() < 3) continue;

        //出勤不可
        QString date = cols[0].trimmed();
        QString person = normalize(cols[1]);
        QString plan = normalize(cols[2]);
        if (date == today && person == targetName)
        {
            if (plan == "有休" || plan == "代休" || plan.startsWith("出張"))
            {
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
}

void MainWindow::loadTodayPlans()
{
    //予定テーブルの基本設定
    ui->TableTodayPlan->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->TableTodayPlan->setFocusPolicy(Qt::NoFocus);
    ui->TableTodayPlan->setSelectionMode(QAbstractItemView::NoSelection);
    ui->TableTodayPlan->setColumnCount(2);
    ui->TableTodayPlan->setHorizontalHeaderLabels(QStringList() << "名前" << "予定");
    ui->TableTodayPlan->setRowCount(0);

    QString today = QDate::currentDate().toString("yyyy/MM/dd");
    QFile file(planPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);
    while (!in.atEnd())
    {
        QStringList cols = in.readLine().split(",");
        if (cols.size() < 3) continue;
        if (cols[0].trimmed() != today) continue;
        int row = ui->TableTodayPlan->rowCount();
        ui->TableTodayPlan->insertRow(row);
        ui->TableTodayPlan->setItem(row, 0, new QTableWidgetItem(cols[1]));
        ui->TableTodayPlan->setItem(row, 1, new QTableWidgetItem(cols[2]));
    }
    file.close();
    //列幅固定
    ui->TableTodayPlan->setColumnWidth(0, 100);//名前
    ui->TableTodayPlan->setColumnWidth(1, 180);//予定
    //行番号を消す
    ui->TableTodayPlan->verticalHeader()->setVisible(false);
    //カーソル非表示
    ui->TableTodayPlan->clearSelection();
}

