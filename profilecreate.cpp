#include "profilecreate.h"
#include "ui_profilecreate.h"

#include <QtAlgorithms>

ProfileCreate::ProfileCreate(SocketJob* socketJob, QWidget *parent) :
    QWidget(parent), ui(new Ui::ProfileCreate),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Настраиваем интерфейс
    ui->cb_role->addItem("-");
    ui->cb_level->addItem("-");

    // Коннектим сигналы
    connect(_socketJob, &SocketJob::signalAddNewUser, this, &ProfileCreate::slotResultNewUser);
}

ProfileCreate::~ProfileCreate()
{
    delete ui;
}

void ProfileCreate::slotRolesAndLevels(QJsonObject *obj)
{
    // Объект содержит в себе два массива: ролей и уровней
    QJsonArray rolesArr  = obj->value("roles").toArray();
    QJsonArray levelsArr = obj->value("levels").toArray();

    // Достаём составляющие массивов и заполняем ими комбобоксы
    for (int i = 0; i < rolesArr.size(); ++i)
        ui->cb_role->addItem(rolesArr[i].toString());

    for (int i = 0; i < levelsArr.size(); ++i)
        if (levelsArr[i].toInt() > 0)
            ui->cb_level->addItem(QString::number(levelsArr[i].toInt()));
}

void ProfileCreate::slotResultNewUser(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Пользователь успешно добавлен в БД!");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }
}

void ProfileCreate::on_b_load_clicked()
{
    // Проводим необходимые проверки
    if (!CheckConditions())
        return;

    // Если у юзера выбран -1 или 0 уровень - устанавливаем его в 1
    if(ui->cb_role->currentText() == "user"
       && (ui->cb_level->currentText() == "0"
           || ui->cb_level->currentText() == "-1"))
        ui->cb_level->setCurrentText("1");

    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("login", ui->le_login->text());
    sendObject.insert("password", ui->le_password->text());
    sendObject.insert("role", ui->cb_role->currentText());
    sendObject.insert("level", ui->cb_level->currentText());

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::addNewUser, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

bool ProfileCreate::CheckConditions()
{
    if(ui->le_login->text() == "")
    {
        ui->l_info->setText("Введите логин.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }
    if(ui->le_password->text() == "")
    {
        ui->l_info->setText("Введите пароль.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }
    if(ui->cb_level->currentText() == "-")
    {
        ui->l_info->setText("Выберите уровень.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }
    if(ui->cb_role->currentText() == "-")
    {
        ui->l_info->setText("Выберите роль.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    return true;
}

void ProfileCreate::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void ProfileCreate::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void ProfileCreate::on_cb_role_currentTextChanged(const QString &arg)
{
    // Сначала удаляем другой возможный специальный индекс
    if (ui->cb_level->currentText().toInt() < 1)
        ui->cb_level->removeItem(ui->cb_level->currentIndex());

    // Если выбрана роль админа - блокируем ему выбор уровня
    //  и устанавливаем значение в 0
    if (arg == "admin")
    {
        // Добавляем 0
        ui->cb_level->addItem("0");
        ui->cb_level->setCurrentText("0");
        ui->cb_level->setEnabled(false);
    }
    // Если выбрана роль модератора - также блокируем ему выбор уровня
    //  и устанавливаем значение в -1
    else if(arg == "moderator")
    {
        // Добавляем -1
        ui->cb_level->addItem("-1");
        ui->cb_level->setCurrentText("-1");
        ui->cb_level->setEnabled(false);
    }
    else
    {
        ui->cb_level->setCurrentText("1");
        ui->cb_level->setEnabled(true);
    }
}


void ProfileCreate::on_le_login_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileCreate::on_le_password_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileCreate::on_cb_role_activated(int index)
{ ClearInfoLabel(); }

void ProfileCreate::on_cb_level_activated(int index)
{
    // Если у юзера выбран -1 или 0 уровень - устанавливаем его в 1
    if(ui->cb_role->currentText() == "user"
       && (ui->cb_level->currentText() == "0"
           || ui->cb_level->currentText() == "-1"))
    {
        ui->cb_level->setCurrentText("1");
        ui->l_info->setText("У роли \"user\" не может быть -1 или 0 уровня доступа.");
        ReloadStyle(ui->l_info, "default");
    }
    else
        ClearInfoLabel();
}

