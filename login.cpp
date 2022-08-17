#include "login.h"
#include "ui_login.h"
#include "commands.h"
#include <QStyle>

Login::Login(SocketJob* socketJob, QWidget *parent) : QWidget(parent), ui(new Ui::Login),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Забиваем стандартные значения в поля подключения
    ui->le_ip->setText("127.0.0.1");
    ui->le_port->setText("1234");

    // Блокируем кнопку входа
    ui->btn_enter->setEnabled(false);

    // Настраиваем отображение lineedit пароля на звёздочки
    ui->le_password->setEchoMode(QLineEdit::Password);

    // Настраиваем интерфейс
    ReloadStyle(ui->groupBox,   "white");
    ReloadStyle(ui->groupBox_2, "white");
    ReloadStyle(ui->l_sysName,  "title");

    // Работаем с логотипом
    QPixmap pixmap;
    pixmap.load("res/logo");
    pixmap = pixmap.scaled(102,102, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);   // Масштабируем фото
    ui->l_logo->setPixmap(pixmap);

    // Коннектим сигналы-слоты
    connect(_socketJob, &SocketJob::signalSuccessfullConnected, this, &Login::slotResultConnect);
    connect(_socketJob, &SocketJob::signalDisconnected, this, &Login::slotDisconnected);
    connect(_socketJob, &SocketJob::signalIdentity, this, &Login::slotResultEnter);
}

Login::~Login()
{    delete ui;    }

void Login::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void Login::slotResultConnect()
{
    // Разблокируем кнопку входа и блокируем кнопку подключения и lineedit'ы инфомрации
    ui->btn_enter->setEnabled(true);
    ui->btn_connect->setEnabled(false);
    ui->le_ip->setEnabled(false);
    ui->le_port->setEnabled(false);

    ui->l_messageServer->setText("Успех!");
    ReloadStyle(ui->l_messageServer, "green");
}

void Login::slotDisconnected()
{
    // Блокируем кнопку входа и разблокируем кнопку подключения и lineedit'ы инфомрации
    ui->btn_enter->setEnabled(false);
    ui->btn_connect->setEnabled(true);
    ui->le_ip->setEnabled(true);
    ui->le_port->setEnabled(true);

    ui->l_messageServer->setText("");
}

void Login::slotResultEnter(QJsonObject* obj)
{
    if (obj->value("access") == true)
    {
        // Запоминаем логин
        _socketJob->SetUserLogin(ui->le_login->text());

        // Очищаем введённые значения и информацию
        ui->le_login->clear();
        ui->le_password->clear();
        ui->l_message->clear();

        // Испускаем сигнал входа в программу
        emit signalSuccessfullEnter();
    }
    else
    {
        // Очищаем строчку с паролем
        ui->le_password->clear();

        // Выводим ошибку на форму
        ui->l_message->setText(obj->value("error").toString());
        ReloadStyle(ui->l_message, "red");
    }
}

void Login::on_btn_connect_clicked()
{
    // Проверяем заполненность полей
    if (ui->le_ip->text() == ""
        || ui->le_port->text() == "")
    {
        ui->l_messageServer->setText("Заполните поля данными!");
        ReloadStyle(ui->l_messageServer, "red");
    }
    else
    {
        // Пытаемся подключиться к серверу
        QString ip  = ui->le_ip->text();
        qint16 port = ui->le_port->text().toInt();
        _socketJob->SocketConnect(ip, port);

        ui->l_messageServer->setText("Подключаемся..");
        ReloadStyle(ui->l_messageServer, "default");
    }
}

void Login::on_btn_enter_clicked()
{
    // Проверяем заполненность полей
    if (ui->le_login->text() == ""
        || ui->le_password->text() == "")
    {
        ui->l_message->setText("Заполните поля данными!");
        ReloadStyle(ui->l_message, "red");
    }
    else
    {
        // Передаём регистрационные данные
        QString login    = ui->le_login->text();
        QString password = ui->le_password->text();

        // Формируем json объект
        QJsonObject object;
        object.insert("login",    login);
        object.insert("password", password);

        // Отправляем объект на сервер
        _socketJob->SendSocketQuery(Commands::identity, &object);
    }
}
