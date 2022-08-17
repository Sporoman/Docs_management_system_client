#include "profilecheck.h"
#include "ui_profilecheck.h"

#include <QStyle>

ProfileCheck::ProfileCheck(SocketJob* socketJob, QWidget *parent) :
    QWidget(parent), ui(new Ui::ProfileCheck),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Настраиваем интерфейс
    ui->l_setId->setText("-");
    ui->l_setFio->setText("-");
    ui->l_setFio->setText("-");
    ui->l_setDate->setText("-");
    ui->l_setPhone->setText("-");
    ui->l_setRole->setText("-");
    ui->l_setLevel->setText("-");
    ui->l_setStatus->setText("-");
    ui->l_photo->setScaledContents(true);

    // Работаем с фотографией
    QPixmap pixmap;
    pixmap.load("res/profile.jpg");
    pixmap = pixmap.scaled(184, 184, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);   // Масштабируем фото
    ui->l_photo->setPixmap(pixmap);

    // Коннектим сигналы
    connect(_socketJob, &SocketJob::signalSearchUserForShow, this, &ProfileCheck::slotResultSearchUser);
}

ProfileCheck::~ProfileCheck()
{
    delete ui;
}

void ProfileCheck::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void ProfileCheck::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void ProfileCheck::slotResultSearchUser(QJsonObject *obj)
{
    // Обрабатываем успешность операции
    bool success = obj->value("success").toBool();

    // Выводим ошибку в случае неудачи
    if (!success)
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");

        return;
    }

    // Оповещаем пользователя
    ui->l_info->setText("Пользователь найден!");
    ReloadStyle(ui->l_info, "green");

    // Распаковываем пакет
    QString id         = QString::number(obj->value("id").toInt());
    QString fio        = obj->value("fio").toString();
    QString phone      = obj->value("phone").toString();
    QString reg_date   = obj->value("reg_date").toString();
    QString role_name  = obj->value("role_name").toString();
    QString level      = QString::number(obj->value("level").toInt());
    QString level_name = obj->value("level_name").toString();
    QString status     = "Отключённый";
    if (obj->value("status").toBool())
        status = "Активный";

    // Работаем с фотографией
    QPixmap pixmap;
    if(obj->value("photo").toString() == "")    // Если фотографии нет, то устанавливаем стандартную
    {
        pixmap.load("res/profile.jpg");
        pixmap = pixmap.scaled(184, 184, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);   // Масштабируем фото
    }
    else
    {
        // Помним, что фото лежит в hex'e
        QString imageFromString(obj->value("photo").toString());
        QByteArray imageFromBytes(QByteArray::fromHex(imageFromString.toUtf8()));
        pixmap.loadFromData(imageFromBytes);
    }
    ui->l_photo->setPixmap(pixmap);

    // Отображаем информацию
    ui->l_setId->setText(id);
    ui->l_setFio->setText(fio);
    ui->l_setPhone->setText(phone);
    ui->l_setDate->setText(reg_date);
    ui->l_setRole->setText(role_name);
    ui->l_setStatus->setText(status);
    ui->l_setLevel->setText(level_name + " (" + level + ")");
}

void ProfileCheck::on_b_search_clicked()
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
    _socketJob->SendSocketQuery(Commands::searchUserForShow, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

void ProfileCheck::on_le_searchId_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

void ProfileCheck::on_le_searchLogin_textEdited(const QString &arg1)
{ ClearInfoLabel(); }

