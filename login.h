#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "socket_job.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE


class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(SocketJob* socketJob, QWidget* parent = 0);
    ~Login();

private:
    Ui::Login* ui;
    SocketJob* _socketJob;

    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotResultConnect();
    void slotDisconnected();
    void slotResultEnter(QJsonObject*);

    void on_btn_connect_clicked();
    void on_btn_enter_clicked();

signals:
    void signalSuccessfullEnter();

};

#endif // LOGIN_H
