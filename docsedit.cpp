#include "docsedit.h"
#include "ui_docsedit.h"
#include "docforshowinfo.h"

DocsEdit::DocsEdit(SocketJob* socketJob, const QString& iconPath, QWidget *parent)
    : QWidget(parent), ui(new Ui::DocsEdit),
      _socketJob(socketJob)
{
    ui->setupUi(this);

    // Инициализируем переменные
    _activeDoc = new ActiveDoc;

    // Настраиваем интерфейс
    ui->cb_levelsSearch->addItem("Все");
    BlockUserInfo();
    ReloadStyle(ui->sa_contents_results, "inside");
    ReloadStyle(ui->l_resultSearch, "manage");

    // Выбираем иконку
    _iconEdit = new QIcon(iconPath + "edit.png");

    // Настраиваем layout для документов
    _layoutDocs = new QVBoxLayout();
    _layoutDocs->setContentsMargins(0,0,0,0);
    _layoutDocs->setSpacing(3);

    // Коннектим сигналы слоты
    connect(_socketJob, &SocketJob::signalGetDocs, this, &DocsEdit::slotGetDocs);
    connect(_socketJob, &SocketJob::signalSendDocFullInfo, this, &DocsEdit::slotSendDocFullInfo);
    connect(_socketJob, &SocketJob::signalEditDocInfo, this, &DocsEdit::slotEditDocInfo);
    connect(_socketJob, &SocketJob::signalDeleteDoc, this, &DocsEdit::slotDeleteDoc);
}

DocsEdit::~DocsEdit()
{
    delete ui;
    delete _iconEdit;
}

void DocsEdit::slotRolesAndLevels(QJsonObject *obj)
{
    // Вытаскиваем массив уровней
    QJsonArray levelsArr = obj->value("levels").toArray();

    // Достаём составляющие массивов и заполняем ими комбобоксы
    for (int i = 0; i < levelsArr.size(); ++i)
        if (levelsArr[i].toInt() > 0)
        {
            ui->cb_levelsSearch->addItem(QString::number(levelsArr[i].toInt()));
            ui->cb_levels->addItem(QString::number(levelsArr[i].toInt()));
        }
}

void DocsEdit::slotSendDocFullInfo(QJsonObject *obj)
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
    ui->l_info->setText("Документ найден!");
    ReloadStyle(ui->l_info, "green");

    // Распаковываем пакет
    QString id        = QString::number(obj->value("id").toInt());
    _activeDoc->name  = obj->value("name").toString();
    _activeDoc->level = QString::number(obj->value("level").toInt());
    QString date      = obj->value("load_date").toString();
    date.chop(7);
    date.replace('T', ' ');

    // Разблокируем поля и обновляем информацию
    UnblockUserInfo();

    ui->l_set_id->setText(id);
    ui->l_set_date->setText(date);
    ui->le_name->setText(_activeDoc->name);
    ui->cb_levels->setCurrentText(_activeDoc->level);
}

void DocsEdit::slotEditDocInfo(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Информация о документе успешно отредактирована!");
        ReloadStyle(ui->l_info, "green");
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }

    // Обновляем активные поля
    _activeDoc->name  = ui->le_name->text();
    _activeDoc->level = ui->cb_levels->currentText();
}

void DocsEdit::slotDeleteDoc(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if(success)
    {
        ui->l_info->setText("Документ успешно удалён!");
        ReloadStyle(ui->l_info, "green");

        // Блокируем поля
        BlockUserInfo();
    }
    else
    {
        ui->l_info->setText(obj->value("error").toString());
        ReloadStyle(ui->l_info, "brown");
    }
}

void DocsEdit::slotGetDocs(QJsonObject *obj)
{
    // Оповещаем пользователя
    ui->l_info->setText("Ответ пришёл!");
    ReloadStyle(ui->l_info, "green");

    // Очищаем виджет лейаута документов
    while (QLayoutItem* item = _layoutDocs->takeAt(0))
    {
        item->widget()->deleteLater();
        delete item;
    }

    // Суем шапку
    DocInfo docInfo{"ID", "Имя", "Уровень"};
    DocForShowInfo* ui_doc = new DocForShowInfo(_socketJob, &docInfo, true, _iconEdit);

    // Добавляем её в layout
    _layoutDocs->addWidget(ui_doc);

    // Объект - это массив документов
    QJsonArray docsArr = obj->value("docs").toArray();

    // Разбиваем массив документов на документы и работаем с ними
    for (int i = 0; i < docsArr.size(); ++i)
    {
        QJsonObject doc(docsArr[i].toObject());

        QString id_doc(QString::number(doc.value("id_doc").toInt()));
        QString name(doc.value("name").toString());
        QString level(QString::number(doc.value("level").toInt()));

        // Создаём виджет документа и заполняем его шаблоном
        DocInfo docInfo{id_doc, name, level};
        DocForShowInfo* ui_doc = new DocForShowInfo(_socketJob, &docInfo, false, _iconEdit);

        // Добавляем его в layout для stackedWidget'а
        _layoutDocs->addWidget(ui_doc);
    }

    // Добавляем стретч для компоновки виджетов и добавляем layout в scrollArea
    _layoutDocs->addStretch();
    ui->sa_contents_results->setLayout(_layoutDocs);
}

void DocsEdit::on_b_search_clicked()
{
    // Если заполнен id документа
    if (ui->le_idSearch->text() != "")
    {
        // Формируем json объект
        QJsonObject object;
        object.insert("id_doc", ui->le_idSearch->text());

        // Отправляем объект на сервер
        _socketJob->SendSocketQuery(Commands::sendDocFullInfo, &object);
    }

    // Получаем текст из лайнедита и уровень из комбобокса
    QString text  = ui->le_nameSearch->text();
    QString level;

    if (ui->cb_levelsSearch->currentIndex() == 0)
        level = "all";
    else
        level = ui->cb_levelsSearch->currentText();

    // Формируем json объект
    QJsonObject object;
    object.insert("text",  text);
    object.insert("level", level);

    // Отправляем запрос на список документов
    _socketJob->SendSocketQuery(Commands::getDocs, &object);


    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

void DocsEdit::BlockUserInfo()
{
    ui->le_name->setEnabled(false);
    ui->cb_levels->setEnabled(false);
}

void DocsEdit::UnblockUserInfo()
{
    ui->le_name->setEnabled(true);
    ui->cb_levels->setEnabled(true);
}

void DocsEdit::ClearInfoLabel()
{
    if (ui->l_info->text() != "")
        ui->l_info->setText("");
}

void DocsEdit::ReloadStyle(QWidget *widget, const QString &name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}


void DocsEdit::on_b_update_clicked()
{
    // Проводим необходимые проверки
    if (!CheckConditions())
        return;

    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("id_doc", ui->l_set_id->text());
    sendObject.insert("name",   ui->le_name->text());
    sendObject.insert("level",  ui->cb_levels->currentText());

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::editDocInfo, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

void DocsEdit::on_b_delete_clicked()
{
    if (ui->l_set_id->text() == "-")
    {
        ui->l_info->setText("Выберите документ.");
        ReloadStyle(ui->l_info, "default");
        return;
    }

    // Формируем пакет для отправки запроса на сервер
    QJsonObject sendObject;

    // Заполяем пакет данными
    sendObject.insert("id_doc", ui->l_set_id->text());

    // Отправляем запрос
    _socketJob->SendSocketQuery(Commands::deleteDoc, &sendObject);

    // Выводим сообщение об отправке
    ui->l_info->setText("Запрос отправлен..");
    ReloadStyle(ui->l_info, "green");
}

bool DocsEdit::CheckConditions()
{
    if(ui->le_name->text() == ""
       && ui->le_idSearch->text() == "")
    {
        ui->l_info->setText("Введите новое название документа.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    if(ui->le_name->text() == _activeDoc->name
       && ui->cb_levels->currentText() == _activeDoc->level)
    {
        ui->l_info->setText("Вы не изменили никакую информацию.");
        ReloadStyle(ui->l_info, "red");

        return false;
    }

    return true;
}

void DocsEdit::on_le_idSearch_textEdited(const QString &arg1)
{   ClearInfoLabel();   }

void DocsEdit::on_le_nameSearch_textEdited(const QString &arg1)
{   ClearInfoLabel();   }

void DocsEdit::on_le_name_textEdited(const QString &arg1)
{   ClearInfoLabel();   }

void DocsEdit::on_cb_levelsSearch_activated(int index)
{   ClearInfoLabel();   }

void DocsEdit::on_cb_levels_activated(int index)
{   ClearInfoLabel();   }

