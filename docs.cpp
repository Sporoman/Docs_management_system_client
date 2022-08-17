#include "docs.h"
#include "ui_docs.h"

#include "commands.h"
#include <QStyle>

Docs::Docs(SocketJob* socketJob, Document* document, bool isTop, QIcon* iconFav, QIcon* iconDownload, QWidget *parent) :
    QWidget(parent), ui(new Ui::Docs),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Запоминаем id документа и его "избранность"
    _idDocument   = document->id;
    _isFavorite   = document->favorite;
    _iconFav      = iconFav;
    _iconDownload = iconDownload;

    // Записываем информацию о документе в лейблы
    ui->l_name->setText("- " + document->name);
    ui->l_level->setText(QString::number(document->level));

    // Проверяем на "шапку"
    if (isTop)
    {
        ui->b_download->hide();
        ui->b_download->setEnabled(false);
        ui->b_favorite->hide();
        ui->b_favorite->setEnabled(false);
        ui->l_level->setText("              Уровень");
        ui->l_name->setText("Имя");
    }
    else
    {
        ui->l_level->setText(QString::number(document->level).leftJustified(9));
        ui->l_name->setText(document->name);
    }

    // Редактируем интерфейс в зависимости от избранности
    SetupSettings();
}

Docs::~Docs()
{
    delete ui;
}

void Docs::SetupSettings()
{
    // Настраивам кнопки
    ui->b_favorite->setIconSize(QSize(40, 30));
    ui->b_favorite->setIcon(*_iconFav);
    ui->b_download->setIconSize(QSize(40, 30));
    ui->b_download->setIcon(*_iconDownload);

    if(_isFavorite)
        ui->b_favorite->setObjectName("favorite");
    else
        ui->b_favorite->setObjectName("");

    ReloadStyle(ui->b_favorite);
}

void Docs::ReloadStyle(QWidget *widget)
{
    style()->unpolish(widget);
    style()->polish(widget);
}

void Docs::on_b_download_clicked()
{
    // Формируем json объект
    QJsonObject object;
    object.insert("id_doc", _idDocument);

    // Запоминаем id документа
    _socketJob->SetLastDocId(_idDocument);

    // Отправляем объект на сервер
    _socketJob->SendSocketQuery(Commands::sendDocInfo, &object);
}

void Docs::on_b_favorite_clicked()
{
    // Формируем json объект
    QJsonObject object;
    object.insert("id_doc", _idDocument);

    // Отправляем запрос в зависимости от избранности
    if(_isFavorite)
        _socketJob->SendSocketQuery(Commands::deleteFavoriteDoc, &object);
    else
        _socketJob->SendSocketQuery(Commands::addFavoriteDoc, &object);

    // Инвертируем показатель избранности и перенастраиваем интерфейс
    _isFavorite = !_isFavorite;
    SetupSettings();

    // Посылаем сигнал на будущее обновление списков документов
    emit signalUpdateDocs();
}

