#ifndef STATISTICS_H
#define STATISTICS_H

#include <QWidget>
#include "socket_job.h"

namespace Ui {
class Statistics;
}

class Statistics : public QWidget
{
    Q_OBJECT

public:
    explicit Statistics(SocketJob* socketJob, QWidget *parent = nullptr);
    ~Statistics();

private slots:
    void slotGetStatistics(QJsonObject*);

    void on_b_getStatistics_clicked();
    void on_b_pdfGenerated_clicked();
    void on_sb_count_valueChanged(int arg1);
    void on_cb_interval_activated(int index);

private:
    Ui::Statistics *ui;
    SocketJob* _socketJob;

    QString _currentDateCount;
    QString _currentDateInterval;

private:
    QString CurrentInterval();
    QString GetPdfInterval();
    void SetChart();
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");
};

#endif // STATISTICS_H
