#include "searcher.h"
#include "ui_searcher.h"

Searcher::Searcher(SocketJob* socketJob, Commands::Command command, QWidget *parent)
    : QWidget(parent), ui(new Ui::Searcher),
    _socketJob(socketJob), _command(command)
{
    ui->setupUi(this);

    // Настраиваем комбо бокс уровней
    ui->cb_levels->addItem("Все");
    for (int i = 1; i <= _socketJob->GetUserLevel(); ++i)
        ui->cb_levels->addItem(QString::number(i));

    // Настраиваем иконки и текст кнопок
    SetButtonIcon(ui->b_search, "res/icons/search.png", " Найти");
    SetButtonIcon(ui->b_update, "res/icons/update.png", " Обновить");
}

Searcher::~Searcher()
{
    delete ui;
}

void Searcher::SetButtonIcon(QToolButton *btn, const QString &iconPath, const QString& text)
{
    QIcon icon(iconPath);
    btn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    btn->setText(text);
    btn->setIconSize(QSize(20,20));
    btn->setIcon(icon);
}

void Searcher::on_b_search_clicked()
{
    // Получаем текст из лайнедита и уровень из комбобокса
    QString text  = ui->le_text->text();
    QString level;

    if (ui->cb_levels->currentIndex() == 0)
        level = "all";
    else
        level = ui->cb_levels->currentText();

    // Формируем json объект
    QJsonObject object;
    object.insert("text",  text);
    object.insert("level", level);

    // Отправляем запрос на список документов
    _socketJob->SendSocketQuery(_command, &object);
}


void Searcher::on_b_update_clicked()
{
    // Сбрасываем gui и повторяем запрос
    ui->le_text->setText("");
    ui->cb_levels->setCurrentIndex(0);

    _socketJob->SendSocketQuery(_command);
}

