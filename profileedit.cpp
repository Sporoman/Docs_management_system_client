#include "profileedit.h"
#include "ui_profileedit.h"

ProfileEdit::ProfileEdit(SocketJob* socketJob, QWidget *parent) :
    QWidget(parent), ui(new Ui::ProfileEdit),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Инициализируем переменные
    _activeUser = new ActiveUser;

    // Настраиваем интерфейс
    ui->cb_role->addItem("-");
    ui->cb_level->addItem("-");

    BlockUserInfo();

    // Коннектим сигналы
    connect(_socketJob, &SocketJob::signalSearchUserForEdit, this, &ProfileEdit::slotResultSearchUser);
    connect(_socketJob, &SocketJob::signalEditUser, this, &ProfileEdit::slotResultEditUser);
    connect(_socketJob, &SocketJob::signalSetUserStatus, this, &ProfileEdit::slotResultSetStatus);
}

ProfileEdit::~ProfileEdit()
{
    delete ui;
}

void ProfileEdit::slotRolesAndLevels(QJsonObject *obj)
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

void ProfileEdit::slotResultSearchUser(QJsonObject *obj)
{
    // Обрабатываем успешность операции
    bool success = obj->value("success").toBool();

    // Выводим ошибку в случае неудачи
    if (!success)
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");

        // Блокируем поля
        BlockUserInfo();
        return;
    }

    // Оповещаем пользователя
    ui->l_info->setText("Пользователь найден!");
    ReloadStyle(ui->l_info, "green");

    // Сначала удаляем возможный специальный индекс
    if (ui->cb_level->currentText().toInt() < 1)
        ui->cb_level->removeItem(ui->cb_level->currentIndex());

    // Распаковываем пакет
    _activeUser->id       = obj->value("id").toInt();
    _activeUser->login    = obj->value("login").toString();
    _activeUser->password = obj->value("password").toString();
    _activeUser->level    = QString::number(obj->value("level").toInt());
    _activeUser->role     = obj->value("role").toString();
    _activeUser->status   = obj->value("status").toBool();

    // Разблокируем поля и обновляем информацию
    UnblockUserInfo();

    ui->le_login->setText(_activeUser->login);
    ui->le_password->setText(_activeUser->password);
    ui->cb_level->setCurrentText(_activeUser->level);
    ui->cb_role->setCurrentText(_activeUser->role);
    if (_activeUser->status)
    {
        ui->l_setStatus->setText("Активный");
        ReloadStyle(ui->l_setStatus, "green");

        ui->b_setStatus->setText("Отключить");
    }
    else
    {
        ui->l_setStatus->setText("Отключённый");
        ReloadStyle(ui->l_setStatus, "brown");

        ui->b_setStatus->setText("Активировать");
    }

    // Настраиваем поля под модератора и админа
    if (_activeUser->role == "admin")
    {
        // Добавляем 0
        ui->cb_level->addItem("0");
        ui->cb_level->setCurrentText("0");
        ui->cb_level->setEnabled(false);
    }
    if (_activeUser->role == "moderator")
    {
        // Добавляем -1
        ui->cb_level->addItem("-1");
        ui->cb_level->setCurrentText("-1");
        ui->cb_level->setEnabled(false);
    }
}

void ProfileEdit::slotResultEditUser(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Информация о пользователе успешно отредактирована!");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }

    // Обновляем активные поля
    _activeUser->login    = ui->le_login->text();
    _activeUser->password = ui->le_password->text();
    _activeUser->level    = ui->cb_level->currentText();
    _activeUser->role     = ui->cb_role->currentText();
}

void ProfileEdit::slotResultSetStatus(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Статус успешно изменён!");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }

    // Обновляем активное поле статуса и обновляем интерфейс
    _activeUser->status = !_activeUser->status;
    if (_activeUser->status)
    {
        ui->l_setStatus->setText("Активный");
        ReloadStyle(ui->l_setStatus, "green");

        ui->b_setStatus->setText("Отключить");
    }
    else
    {
        ui->l_setStatus->setText("Отключённый");
        ReloadStyle(ui->l_setStatus, "brown");

        ui->b_setStatus->setText("Активировать");
    }
}

void ProfileEdit::on_b_search_clicked()
{
    // Проверяем поля на заполненность
    if (ui->le_searchId->text() == "" && ui->le_searchLogin->text() == "")
    {
        ui->l_info->setText("Заполните хотя бы одно поле для поиска");
        ReloadStyle(ui->l_info, "default");
        return;
    }

    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("id", ui->le_searchId->text());
    sendObject.insert("login", ui->le_searchLogin->text());

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::searchUserForEdit, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

void ProfileEdit::on_b_update_clicked()
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
    sendObject.insert("id", _activeUser->id);
    sendObject.insert("login", ui->le_login->text());
    sendObject.insert("password", ui->le_password->text());
    sendObject.insert("role", ui->cb_role->currentText());
    sendObject.insert("level", ui->cb_level->currentText());

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::editUser, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

void ProfileEdit::on_b_setStatus_clicked()
{
    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("id", _activeUser->id);

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::setUserStatus, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

void ProfileEdit::on_cb_level_activated(int index)
{
    // Если у юзера выбран -1 или 0 уровень - устанавливаем его в 1
    if(ui->cb_role->currentText() == "user"
       && (ui->cb_level->currentText() == "0"
           || ui->cb_level->currentText() == "-1"))
    {
        ui->cb_level->setCurrentText("1");
        ui->l_info->setText("У роли \"user\" не может быть 0 или -1 уровня доступа.");
        ReloadStyle(ui->l_info, "default");
    }
    else
        ClearInfoLabel();
}

void ProfileEdit::on_cb_role_currentTextChanged(const QString &arg)
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
        if (_activeUser->level.toInt() > 0)
            ui->cb_level->setCurrentText(_activeUser->level);
        else
            ui->cb_level->setCurrentText("1");
        ui->cb_level->setEnabled(true);
    }
}

void ProfileEdit::BlockUserInfo()
{
    ui->le_login->setEnabled(false);
    ui->le_password->setEnabled(false);
    ui->cb_level->setEnabled(false);
    ui->cb_role->setEnabled(false);
    ui->b_update->setEnabled(false);
    ui->b_setStatus->setEnabled(false);
}

void ProfileEdit::UnblockUserInfo()
{
    ui->le_login->setEnabled(true);
    ui->le_password->setEnabled(true);
    ui->cb_level->setEnabled(true);
    ui->cb_role->setEnabled(true);
    ui->b_update->setEnabled(true);
    ui->b_setStatus->setEnabled(true);
}

bool ProfileEdit::CheckConditions()
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

    if(ui->le_login->text() == _activeUser->login
       && ui->le_password->text() == _activeUser->password
       && ui->cb_level->currentText() == _activeUser->level
       && ui->cb_role->currentText() == _activeUser->role)
    {
        ui->l_info->setText("Вы не изменили никакую информацию.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    return true;
}

void ProfileEdit::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void ProfileEdit::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void ProfileEdit::on_le_searchId_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileEdit::on_le_searchLogin_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileEdit::on_le_login_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileEdit::on_le_password_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileEdit::on_cb_role_activated(int index)
{ ClearInfoLabel(); }

