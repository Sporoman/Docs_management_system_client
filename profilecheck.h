#ifndef PROFILECHECK_H
#define PROFILECHECK_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class ProfileCheck; }


class ProfileCheck : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileCheck(SocketJob* socketJob, QWidget *parent = nullptr);
    ~ProfileCheck();

private:
    Ui::ProfileCheck *ui;
    SocketJob* _socketJob;

private:
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotResultSearchUser(QJsonObject*);

    void on_b_search_clicked();
    void on_le_searchId_textEdited(const QString &arg1);
    void on_le_searchLogin_textEdited(const QString &arg1);
};

#endif // PROFILECHECK_H
