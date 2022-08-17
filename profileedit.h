#ifndef PROFILEEDIT_H
#define PROFILEEDIT_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class ProfileEdit; }

struct ActiveUser;


class ProfileEdit : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileEdit(SocketJob* socketJob, QWidget *parent = nullptr);
    ~ProfileEdit();

public slots:
    void slotRolesAndLevels(QJsonObject*);

private:
    Ui::ProfileEdit *ui;
    SocketJob* _socketJob;

    ActiveUser* _activeUser;

private:
    void BlockUserInfo();
    void UnblockUserInfo();
    bool CheckConditions();
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotResultSearchUser(QJsonObject*);
    void slotResultEditUser(QJsonObject*);
    void slotResultSetStatus(QJsonObject*);

    void on_b_search_clicked();
    void on_b_update_clicked();
    void on_cb_level_activated(int index);
    void on_b_setStatus_clicked();
    void on_cb_role_currentTextChanged(const QString &arg1);
    void on_le_searchId_textEdited(const QString &arg1);
    void on_le_searchLogin_textEdited(const QString &arg1);
    void on_le_login_textEdited(const QString &arg1);
    void on_le_password_textEdited(const QString &arg1);
    void on_cb_role_activated(int index);
};

struct ActiveUser
{
    int id;
    QString login;
    QString password;
    QString level;
    QString role;
    bool status;
};

#endif // PROFILEEDIT_H
