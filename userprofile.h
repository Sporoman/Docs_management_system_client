#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class userProfile; }

struct ActiveInfo;


class UserProfile : public QWidget
{
    Q_OBJECT

public:
    explicit UserProfile(SocketJob* socketJob, QWidget *parent = nullptr);
    ~UserProfile();

private:
    Ui::userProfile *ui;
    SocketJob* _socketJob;

    ActiveInfo* _activeInfo;
    bool isPhotoDefault;

private:
    bool CheckConditions();
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotResultUserInfo(QJsonObject*);
    void slotResultEditUserInfo(QJsonObject*);

    void on_b_update_clicked();
    void on_b_photo_clicked();
    void on_le_fio_textEdited(const QString &arg1);
    void on_le_phone_textEdited(const QString &arg1);
};

struct ActiveInfo
{
    QString name;
    QString phone;
    bool photo;
};

#endif // USERPROFILE_H
