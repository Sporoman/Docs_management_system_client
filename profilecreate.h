#ifndef PROFILECREATE_H
#define PROFILECREATE_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class ProfileCreate; }

class ProfileCreate : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileCreate(SocketJob* socketJob, QWidget *parent = nullptr);
    ~ProfileCreate();

public slots:
    void slotRolesAndLevels(QJsonObject*);

private:
    Ui::ProfileCreate *ui;
    SocketJob* _socketJob;

private:
    bool CheckConditions();
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotResultNewUser(QJsonObject*);

    void on_b_load_clicked();
    void on_cb_role_currentTextChanged(const QString &arg1);
    void on_le_login_textEdited(const QString &arg1);
    void on_le_password_textEdited(const QString &arg1);
    void on_cb_role_activated(int index);
    void on_cb_level_activated(int index);
};

#endif // PROFILECREATE_H
