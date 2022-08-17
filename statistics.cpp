#include "statistics.h"
#include "ui_statistics.h"

#include <QStyle>
#include <QFileDialog>
#include <QPrinter>
#include <QPieSeries>
#include <QChart>
#include <QChartView>

Statistics::Statistics(SocketJob* socketJob, QWidget *parent) :
    QWidget(parent), ui(new Ui::Statistics),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Настраиваем интерфейс
    ui->cb_interval->addItem("Дни");
    ui->cb_interval->addItem("Недели");
    ui->cb_interval->addItem("Месяца");
    ui->cb_interval->addItem("Года");
    ui->cb_interval->addItem("За всё время");

    // Блокируем кнопку генерации отчёта
    ui->b_pdfGenerated->setEnabled(false);

    // Коннектим сигналы-слоты
    connect(_socketJob, &SocketJob::signalGetStatistics, this, &Statistics::slotGetStatistics);
}

Statistics::~Statistics()
{
    delete ui;
}

void Statistics::slotGetStatistics(QJsonObject *obj)
{
    // Оповещаем пользователя
    ui->l_info->setText("Ответ пришёл!");
    ReloadStyle(ui->l_info, "green");

    // Запоминаем выбранные настройки временного интервала
    _currentDateCount    = ui->sb_count->text();
    _currentDateInterval = ui->cb_interval->currentText();

    // Разблокируем кнопку генерации отчёта
    ui->b_pdfGenerated->setEnabled(true);

    // Распаковываем пакет
    QString downloadDoc       = QString::number(obj->value("download_doc").toInt());
    QString addFavFoc         = QString::number(obj->value("add_fav_doc").toInt());
    QString removeFavDoc      = QString::number(obj->value("remove_fav_doc").toInt());
    QString updateInfoDoc     = QString::number(obj->value("update_info_doc").toInt());
    QString deleteDoc         = QString::number(obj->value("delete_doc").toInt());
    QString uploadDoc         = QString::number(obj->value("upload_doc").toInt());
    QString updateInfoProfile = QString::number(obj->value("update_info_profile").toInt());
    QString updateInfoAccount = QString::number(obj->value("update_info_account").toInt());
    QString createAccount     = QString::number(obj->value("create_account").toInt());
    QString setActiveOn       = QString::number(obj->value("set_active_on").toInt());
    QString setActiveOff      = QString::number(obj->value("set_active_off").toInt());

    // Отображаем информацию
    ui->l_d_set_dowload->setText(downloadDoc);
    ui->l_d_set_addFav->setText(addFavFoc);
    ui->l_d_set_delFav->setText(removeFavDoc);
    ui->l_d_set_update->setText(updateInfoDoc);
    ui->l_d_set_upload->setText(uploadDoc);
    ui->l_d_set_delete->setText(deleteDoc);

    ui->l_a_set_updateInfo->setText(updateInfoProfile);
    ui->l_a_set_updateSpecInfo->setText(updateInfoAccount);
    ui->l_a_set_create->setText(createAccount);
    ui->l_a_set_activeOff->setText(setActiveOff);
    ui->l_a_set_activeOn->setText(setActiveOn);

    // Генерируем диаграмму
    //SetChart();
}


void Statistics::on_b_getStatistics_clicked()
{
    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("count_of_interval", ui->sb_count->text());
    sendObject.insert("interval", CurrentInterval());

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::getStatistics, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

QString Statistics::CurrentInterval()
{
    QString interval(ui->cb_interval->currentText());
    if(interval == "Дни")
        return "day";
    else if(interval == "Недели")
        return "week";
    else if(interval == "Месяца")
        return "month";
    else if(interval == "Года")
        return "year";
    else if(interval == "За всё время")
        return "all";

    return "all";
}

QString Statistics::GetPdfInterval()
{
    static const QString strMassIntervalsNames[4][3]
    {
        {"день", "дня", "дней"},
        {"неделю", "недели", "недель"},
        {"месяц", "месяца", "месяцев"},
        {"год", "года", "лет"}
    };
    int interval = 0;   // Для массива выше
    int count = _currentDateCount.toInt();

    // Проверяем счётчик, если он равен 0, значит статистика за всё время
    if (_currentDateCount == "0"
        || _currentDateInterval == "За всё время")
        return "всё время";

    // Выбираем интервал
    if(_currentDateInterval == "Дни")
        interval = 0;
    else if(_currentDateInterval == "Недели")
        interval = 1;
    else if(_currentDateInterval == "Месяца")
        interval = 2;
    else if(_currentDateInterval == "Года")
        interval = 3;

    // Рассчитываем правильный вариант названия временного интервала
    QString reslutDateInterval = strMassIntervalsNames[interval][2];

    if ((count % 100) == 11 || (count % 100) == 12|| (count % 100) == 13 || (count % 100) == 14)
        reslutDateInterval = strMassIntervalsNames[interval][2];
    else if ((count % 10) == 2 || (count % 10) == 3 || (count % 10) == 4)
        reslutDateInterval = strMassIntervalsNames[interval][1];
    else if ((count % 10) == 1)
        reslutDateInterval = strMassIntervalsNames[interval][0];

    QString result = _currentDateCount + " " + reslutDateInterval;
    return result;
}

void Statistics::SetChart()
{
//    ui->l_d_set_dowload->setText(downloadDoc);
//    ui->l_d_set_addFav->setText(addFavFoc);
//    ui->l_d_set_delFav->setText(removeFavDoc);
//    ui->l_d_set_update->setText(updateInfoDoc);
//    ui->l_d_set_upload->setText(uploadDoc);
//    ui->l_d_set_delete->setText(deleteDoc);

//    ui->l_a_set_updateInfo->setText(updateInfoProfile);
//    ui->l_a_set_updateSpecInfo->setText(updateInfoAccount);
//    ui->l_a_set_create->setText(createAccount);
//    ui->l_a_set_activeOff->setText(setActiveOff);
//    ui->l_a_set_activeOn->setText(setActiveOn);

//    QPieSeries *series = new QPieSeries();

//    QPieSlice *slice = new QPieSlice(ui->l_d_addFav->text(), ui->l_d_set_addFav->text().toInt());
//    series->append(slice);

//    QPieSlice *slice2 = new QPieSlice(ui->l_d_delFav->text(), ui->l_d_set_delFav->text().toInt());
//    series->append(slice2);

//    QChart* chart = new QChart();
//    chart->addSeries(series);
//    chart->setTitle("Диаграмма");
//    chart->legend()->setAlignment(Qt::AlignRight);
//    chart->setAnimationOptions(QChart::AllAnimations);

//    QChartView *chartView = new QChartView(chart);
//    chartView->setRenderHint(QPainter::Antialiasing);

//    // Очищаем виджет лейаута документов
//    while (QLayoutItem* item = ui->layout_chart->takeAt(0))
//    {
//        item->widget()->deleteLater();
//        delete item;
//    }

//    ui->layout_chart->addWidget(chartView);
}

void Statistics::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void Statistics::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void Statistics::on_b_pdfGenerated_clicked()
{
    QDateTime dateTime(QDateTime::currentDateTime());

    // Имя файла
    QString docName("Отчёт по статистике " + dateTime.toString("dd-MM-yyyy") + ".pdf");

    // Получаем путь сохранения файл
    QString fileName(QFileDialog::getSaveFileName(this, "Сохранить документ как", docName));

    // Если место для сохранения файла не выбрано
    if (fileName == "")
    {
        // Выводим сообщение
        ui->l_info->setText("Генерация отчёта отменена.");
        ReloadStyle(ui->l_info, "default");
        return;
    }


    QString strPdf("<html><head><title>Отчёт</title></head><body><tbody>"
    "<h1 align = center style= color:Black>Отчёт по статистике от " + dateTime.date().toString("dd.MM.yyyy") + "</h1>"
    "<div><p> Статистика за " + GetPdfInterval() + ":</p></div>"
    "<div><p><table border = 1>"
        "<tr><td colspan = 2 align = center><b>Документы</b></td></tr>"
        "<tr><td width=200>Скачано</td><td width=\"40\"; align=center>" + ui->l_d_set_dowload->text() + "</td></tr>"
        "<tr><td>Загружено</td><td align = center>" + ui->l_d_set_upload->text() + "</td></tr>"
        "<tr><td>Удалено</td><td align = center>" + ui->l_d_set_delete->text() + "</td></tr>"
        "<tr><td>В \"избранное\"</td><td align = center>" + ui->l_d_set_addFav->text() + "</td></tr>"
        "<tr><td>Из \"избранного\"</td><td align = center>" + ui->l_d_set_delFav->text() + "</td></tr>"
        "<tr><td>Обновлено</td><td align = center>" + ui->l_d_set_update->text() + "</td></tr>"
    "</table></p></div>"
    "<div><p><table border = 1>"
        "<tr><td colspan = 2 align = center><b>Аккаунты</b></td></tr>"
        "<tr><td width=200>Обновлений профиля</td><td width=\"40\"; align=center>" + ui->l_a_set_updateInfo->text() + "</td></tr>"
        "<tr><td>Изменений админами</td><td align = center>" + ui->l_a_set_updateSpecInfo->text() + "</td></tr>"
        "<tr><td>Создано</td><td align = center>" + ui->l_a_set_create->text() + "</td></tr>"
        "<tr><td>Отключено</td><td align = center>" + ui->l_a_set_activeOff->text() + "</td></tr>"
        "<tr><td>Активировано</td><td align = center>" + ui->l_a_set_activeOn->text() + "</td></tr>"
    "</table></p></div>"
    "<div><p>Документ сформирован в " + dateTime.time().toString("hh:mm:ss") + " пользователем '" + _socketJob->GetUserLogin() + "'.</p></div>"
    "</tbody></body></html>"
    );

    QPrinter pdfPrinter;
    pdfPrinter.setOutputFormat(QPrinter::PdfFormat);
    pdfPrinter.setColorMode(QPrinter::Color);
    pdfPrinter.setFullPage(true);

    QTextDocument pdfDoc;
    pdfDoc.setDocumentMargin(0);
    pdfDoc.setHtml(strPdf);
    pdfPrinter.setOutputFileName(fileName);
    pdfDoc.print(&pdfPrinter);

    // Выводим сообщение об успехе
    ui->l_info->setText("Отчёт сгенерирован!");
    ReloadStyle(ui->l_info, "green");
}

void Statistics::on_sb_count_valueChanged(int arg1)
{   ClearInfoLabel();   }

void Statistics::on_cb_interval_activated(int index)
{   ClearInfoLabel();   }
