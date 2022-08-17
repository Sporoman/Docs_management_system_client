#include "upload.h"
#include "ui_upload.h"

#include "commands.h"
#include <QFile>
#include <QFileDialog>
#include <QStyle>

Upload::Upload(SocketJob* socketJob, QWidget *parent)
    : QWidget(parent), ui(new Ui::Upload),
      _socketJob(socketJob)
{
    ui->setupUi(this);

    // Настраиваем спинбокс под level пользователя
    ui->spinBox_level->setRange(1, _socketJob->GetUserLevel());
    ui->spinBox_level->setValue(_socketJob->GetUserLevel());

    // Настраиваем интерфейс
    ui->l_docInfo->setText("Файл не выбран");
    ui->le_name->setEnabled(false);

    // Коннектим сигналы-слоты
    connect(_socketJob, &SocketJob::signalSendDocToServer, this, &Upload::slotResultUpload);
}

Upload::~Upload()
{
    delete ui;
}

void Upload::on_b_chooseFile_clicked()
{
    // Получаем имя файла
    _fileName = QFileDialog::getOpenFileName(this, tr("Выбрать документ для загрузки"));
    if (_fileName == "")
    {
        // Информируем пользователя
        ui->l_docInfo->setObjectName("");
        ui->l_docInfo->setStyleSheet(ui->l_docInfo->styleSheet());
        ui->l_docInfo->setText("Файл не выбран");
        ReloadStyle(ui->l_docInfo, "default");

        // Блокируем и очищаем line edit для имени файла
        ui->le_name->setEnabled(false);
        ui->le_name->setText("");

        ClearInfoLabel();
        return;
    }

    // Информируем пользователя
    ui->l_docInfo->setText(QString("Файл готов!"));
    ReloadStyle(ui->l_docInfo, "green");

    // Не пугаться! Просто изымаем имя файла из пути при помощи определения последнего слеша
    ui->le_name->setText(_fileName.right(_fileName.size() - _fileName.lastIndexOf("/") - 1));

    // Разблокируем line edit для имени файла
    ui->le_name->setEnabled(true);

    // Очищаем информационную строку
    ClearInfoLabel();
}

void Upload::on_b_upload_clicked()
{
    // Проводим необходимые проверки
    if (!CheckConditions())
        return;

    // Открываем файл и переводим его в вид для отправки
    QFile file(_fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        // Проверяем размер файла (не больше 20 мб)
        if ((file.size() / 1024.0 / 1024.0) > 20.0)
        {
            ui->l_info->setText("Файл больше 20 мб.");
            ReloadStyle(ui->l_info, "red");
            return;
        }

        // Формируем пакет для отправки документа на сервер
        QJsonObject sendObject;

        // Переводим документ в массив байт
        QByteArray byteArray = file.readAll();

        // Вот тут внимательно. Переводим байты в base64 и засовываем их стринг,
        // чтобы поместить всё это в json-объект
        sendObject.insert("document", QString(byteArray.toBase64()));

        // Пакуем необходимую дополнительную информацию о документе
        sendObject.insert("name", ui->le_name->text());
        sendObject.insert("level", ui->spinBox_level->value());

        // Отправляем запрос
        _socketJob->SendSocketQuery(Commands::sendDocToServer, &sendObject);

        // Закрываем файл
        file.close();

        // Выводим сообщение об отправке
        ui->l_info->setText("Запрос отправлен..");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText("Не удалось открыть файл.");
        ReloadStyle(ui->l_info, "red");
    }
}

bool Upload::CheckConditions()
{
    // Проводим проверку на выбор документа
    if (_fileName.isEmpty())
    {
        ui->l_info->setText("Не выбран документ.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    // Проводим проверку на заполненность имени файла
    if(ui->le_name->text() == "")
    {
        ui->l_info->setText("Не заполнено имя файла.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    return true;
}

void Upload::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void Upload::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void Upload::slotResultUpload(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Документ успешно загружен на сервер!");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }
}

void Upload::on_le_name_textEdited(const QString &arg)
{   ClearInfoLabel();   }

