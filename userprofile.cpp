#include "userprofile.h"
#include "ui_userprofile.h"

#include "commands.h"

#include <QStyle>
#include <QBuffer>
#include <QFileDialog>

UserProfile::UserProfile(SocketJob* socketJob, QWidget *parent)
    : QWidget(parent), ui(new Ui::userProfile),
    _socketJob(socketJob), isPhotoDefault(true)
{
    ui->setupUi(this);

    // Инициализируем переменные
    _activeInfo = new ActiveInfo;

    // Настраиваем ui
    ui->l_photo->setScaledContents(true);   // Улучшаем качество фотографий

    // Коннектим сигналы-слоты
    connect(_socketJob, &SocketJob::signalUserInfo, this, &UserProfile::slotResultUserInfo);
    connect(_socketJob, &SocketJob::signalEditUserInfo, this, &UserProfile::slotResultEditUserInfo);

    // Посылаем запрос на сокет о данных пользователя
    _socketJob->SendSocketQuery(Commands::userInfo);
}

UserProfile::~UserProfile()
{
    delete ui;
}

void UserProfile::slotResultUserInfo(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        // Заполняем интерфейс
        ui->le_fio->setText(obj->value("fio").toString());
        ui->le_phone->setText(obj->value("phone").toString());
        ui->l_setRegDate->setText(obj->value("reg_date").toString());
        ui->l_setRole->setText(obj->value("role_name").toString());

        QString levelName;
        if (_socketJob->GetUserRole() != 2) // Если роль не пользователь, то не нужно показывать уровень доступа
            levelName = obj->value("levelName").toString();
        else
            levelName = obj->value("levelName").toString() + " (" + obj->value("level").toString() + ")";

        ui->l_setLevel->setText(levelName);

        // Работаем с фотографией
        QPixmap pixmap;
        if(obj->value("photo").toString() == "")    // Если фотографии нет, то устанавливаем стандартную
        {
            pixmap.load("res/profile.jpg");
            pixmap = pixmap.scaled(150, 55, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);   // Масштабируем фото
        }
        else
        {
            // Помним, что фото лежит в hex'e
            QString imageFromString(obj->value("photo").toString());
            QByteArray imageFromBytes(QByteArray::fromHex(imageFromString.toUtf8()));
            pixmap.loadFromData(imageFromBytes);

            // Устанавливает флаг стандартной фотографии в false
            isPhotoDefault = false;
        }
        ui->l_photo->setPixmap(pixmap);

        // Запоминаем редактируемую информацию
        _activeInfo->name  = obj->value("fio").toString();
        _activeInfo->phone = obj->value("phone").toString();
        _activeInfo->photo = false;
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }
}

void UserProfile::slotResultEditUserInfo(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Информация успешно обновлена!");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }

    // Обновляем активные поля
    _activeInfo->name  = ui->le_fio->text();
    _activeInfo->phone = ui->le_phone->text();
    _activeInfo->photo = false;
}

void UserProfile::on_b_update_clicked()
{
    // Проводим необходимые проверки
    if (!CheckConditions())
        return;

    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("fio", ui->le_fio->text());
    sendObject.insert("phone", ui->le_phone->text());

    // Упаковываем фотографию
    if (_activeInfo->photo)    // Если фото изменялось
    {
        sendObject.insert("photo_change", true);

        QByteArray imageFromBytes;
        QBuffer inBuffer( &imageFromBytes );
        ui->l_photo->pixmap().save( &inBuffer, "PNG" );

        sendObject.insert("photo", QString(imageFromBytes.toHex()));
    }
    else // Если фото не изменялось
        sendObject.insert("photo_change", false);

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::editUserInfo, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

bool UserProfile::CheckConditions()
{
    if(ui->le_fio->text() == "")
    {
        ui->l_info->setText("Введите ФИО.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }
    if(ui->le_phone->text() == "")
    {
        ui->l_info->setText("Введите Телефон.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }
    if(ui->le_fio->text() == _activeInfo->name
       && ui->le_phone->text() == _activeInfo->phone
       && _activeInfo->photo == false)
    {
        ui->l_info->setText("Вы не изменили никакую информацию.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    return true;
}

void UserProfile::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void UserProfile::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void UserProfile::on_b_photo_clicked()
{
    QFileDialog file_dialog;
    QString file_path = file_dialog.getOpenFileName(0, "Выбор фотографии", QString(),"*png *jpeg *jpg");

    if(file_path != "")
    {
        QPixmap pixmap;
        pixmap.load(file_path);
        pixmap = pixmap.scaled(184, 184, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);   // Масштабируем фото
        ui->l_photo->setPixmap(pixmap);

        // Не забываем отмечать редакцию
        _activeInfo->photo = true;

        // И устанавить флаг стандартной фотографии в false
        isPhotoDefault = false;
    }

    ClearInfoLabel();
}

void UserProfile::on_le_fio_textEdited(const QString &arg1)
{   ClearInfoLabel();   }

void UserProfile::on_le_phone_textEdited(const QString &arg1)
{   ClearInfoLabel();   }

