#ifndef DOCFORSHOWINFO_H
#define DOCFORSHOWINFO_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class DocForShowInfo; }

struct DocInfo;


class DocForShowInfo : public QWidget
{
    Q_OBJECT

public:
    explicit DocForShowInfo(SocketJob* socketJob, DocInfo* docInfo, bool isTop, QIcon* icon, QWidget *parent = nullptr);
    ~DocForShowInfo();

private slots:
    void on_b_info_clicked();

private:
    Ui::DocForShowInfo *ui;
    SocketJob *_socketJob;
};

struct DocInfo
{
    QString id;
    QString name;
    QString level;
};

#endif // DOCFORSHOWINFO_H
