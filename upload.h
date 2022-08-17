#ifndef UPLOAD_H
#define UPLOAD_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class Upload; }

class Upload : public QWidget
{
    Q_OBJECT

public:
    explicit Upload(SocketJob* socketJob, QWidget *parent = nullptr);
    ~Upload();

private:
    Ui::Upload *ui;
    SocketJob* _socketJob;
    QString _fileName;

private:
    bool CheckConditions();
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotResultUpload(QJsonObject*);

    void on_b_chooseFile_clicked();
    void on_b_upload_clicked();
    void on_le_name_textEdited(const QString &arg1);
};

#endif // UPLOAD_H
