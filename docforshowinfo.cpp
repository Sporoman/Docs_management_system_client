#include "docforshowinfo.h"
#include "ui_docforshowinfo.h"

DocForShowInfo::DocForShowInfo(SocketJob* socketJob, DocInfo* docInfo, bool isTop, QIcon* icon, QWidget *parent) :
    QWidget(parent), ui(new Ui::DocForShowInfo),
    _socketJob(socketJob)
{
    ui->setupUi(this);

    // Настраиваем интерфейс
    ui->l_id->setText(docInfo->id.leftJustified(5));

    if (isTop)
    {
        ui->b_info->hide();
        ui->l_level->setText(docInfo->level);
        ui->l_name->setAlignment(Qt::AlignHCenter);
    }
    else
    {
        ui->l_name->setText(docInfo->name);
        ui->l_level->setText("      " + docInfo->level);
    }

    // Настраивам кнопки
    ui->b_info->setIconSize(QSize(20, 20));
    ui->b_info->setIcon(*icon);
}

DocForShowInfo::~DocForShowInfo()
{
    delete ui;
}

void DocForShowInfo::on_b_info_clicked()
{
    // Формируем json объект
    QJsonObject object;
    object.insert("id_doc", ui->l_id->text());

    // Отправляем объект на сервер
    _socketJob->SendSocketQuery(Commands::sendDocFullInfo, &object);
}

