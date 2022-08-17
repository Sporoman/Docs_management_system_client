#ifndef SEARCHER_H
#define SEARCHER_H

#include <QToolButton>
#include <QWidget>
#include "socket_job.h"

namespace Ui { class Searcher; }

class Searcher : public QWidget
{
    Q_OBJECT

public:
    explicit Searcher(SocketJob* socketJob, Commands::Command command, QWidget *parent = nullptr);
    ~Searcher();

private:
    Ui::Searcher *ui;
    SocketJob* _socketJob;
    Commands::Command _command;

    void SetButtonIcon(QToolButton* btn, const QString& iconPath, const QString& text);

private slots:
    void on_b_search_clicked();
    void on_b_update_clicked();
};

#endif // SEARCHER_H
