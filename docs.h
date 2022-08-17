#ifndef DOCS_H
#define DOCS_H

#include <QWidget>
#include "socket_job.h"

namespace Ui { class Docs; }

struct Document;


class Docs : public QWidget
{
    Q_OBJECT

public:
    explicit Docs(SocketJob* socketJob, Document* document, bool isTop, QIcon* iconFav, QIcon* iconDownload, QWidget *parent = nullptr);
    ~Docs();

private:
    Ui::Docs *ui;

    SocketJob* _socketJob;
    int _idDocument;
    bool _isFavorite;
    QIcon* _iconFav;
    QIcon* _iconDownload;

private:
    void SetupSettings();
    void ReloadStyle(QWidget *widget);

private slots:
    void on_b_download_clicked();
    void on_b_favorite_clicked();

signals:
    void signalUpdateDocs();
};


struct Document
{
    int id;
    QString name;
    int level;
    bool favorite;
};

#endif // DOCS_H
