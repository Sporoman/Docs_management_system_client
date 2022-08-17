#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStyle>
#include <QFile>
#include <QFileDialog>
#include "commands.h"
#include "docs.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
    _activeButton(nullptr), _defaultLabelPath("ВСД"), _iconsPath("res/icons/"),
    _messageTime(3000), _checkLeftMenu(false), _checkTopMenu(false), _isNeedUpdateListsDocs(false)
{
    ui->setupUi(this);
    this->setWindowTitle("Информационная система документооборота предприятия");

    // Создаём объект сокета
    _socketJob  = new SocketJob();

    // Создаём и настраиваем форму входа
    _uiLogin = new Login(_socketJob);
    connect(_uiLogin, &Login::signalSuccessfullEnter, this, &MainWindow::slotEnter);

    // Убираем все виджеты и спейсеры с основной формы
    HideAllLayouts();
    HideAllSpacers();

    // Настраиваем стиль надписей
    ReloadStyle(ui->l_path,     "manage");
    ReloadStyle(ui->l_sysName,  "manage");
    ReloadStyle(ui->l_userName, "manage");

    // Добавляем и показываем форму логина в стаке виджетов
    ui->stackedWidget->addWidget(_uiLogin);
    ui->stackedWidget->setCurrentWidget(_uiLogin);
    ReloadStyle(ui->stackedWidget, "likewidget");

    // Обрабатываем общие сигналы от сервера
    connect(_socketJob, &SocketJob::signalError, this, &MainWindow::slotError);
    connect(_socketJob, &SocketJob::signalDisconnected, this, &MainWindow::slotDisconnect);
    connect(_socketJob, &SocketJob::signalSuccessfullConnected, this, &MainWindow::slotConnected);
}

MainWindow::~MainWindow()
{
    delete ui;
    _socketJob->deleteLater();
    _uiLogin->deleteLater();

}

void MainWindow::slotEnter()
{
    // Подгружаем меню
    ShowLayout(ui->lay_topMenu);
    ShowLayout(ui->lay_topMenuExtra);
    ShowMainSpacers();
    ReloadStyle(ui->stackedWidget, "default");

    // В зависимости от роли, инициализируем и загружаем необходимое
    switch (_socketJob->GetUserRole())
    {
        default:
        case 2:
        {
            SetupSettingsForUser();
            ShowLayout(ui->lay_leftMenuUser);
            ShowRoleSpacers(ui->spacer_left_top_user, ui->spacer_left_down_user);
            break;
        }
        case 1:
        {
            SetupSettingsForAdmin();
            ShowLayout(ui->lay_leftMenuAdmin);
            ShowRoleSpacers(ui->spacer_left_top_admin, ui->spacer_left_down_admin);
            break;
        }
        case 3:
        {
            SetupSettingsForModerator();
            ShowLayout(ui->lay_leftMenuModerator);
            ShowRoleSpacers(ui->spacer_left_top_moder, ui->spacer_left_down_moder);
            break;
        }
    }

    // Инициализируем общий профиль пользователя и добавляем его в stacked widget
    _uiUser = new UserProfile(_socketJob);
    ui->stackedWidget->addWidget(_uiUser);

    // Ставим виджет и путь по умолчанию
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    SetDefaultPathForLabel();

    // Устанавливем логин пользователя
    ui->l_userName->setText("Вы авторизованы как: " + _socketJob->GetUserLogin());

    // Устанавливаем логотип
    QPixmap pixmap;
    pixmap.load("res/logo_mini");
    pixmap = pixmap.scaled(50, 50, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);   // Масштабируем лого
    ui->l_logo->setPixmap(pixmap);
}

void MainWindow::slotConnected()
{
    // Оповещаем пользователя
    ui->statusbar->showMessage("Подключение к серверу успешно!", _messageTime);
}

void MainWindow::slotDisconnect()
{
    // Оповещаем пользователя
    ui->statusbar->showMessage("Соединение с сервером нарушено... Попробуйте переподключиться вручную...");
}

void MainWindow::slotError(QJsonObject *obj)
{
    QString errorText = obj->value("error").toString();

    // Оповещаем пользователя
    ui->statusbar->showMessage("Ошибка: " + errorText, _messageTime + 2000);
}

void MainWindow::SetupSettingsForUser()
{
    // Инициализируем формы для пользователя
    InitializeFormsForUser();

    // Коннектим сигналы-слоты
    connect(_socketJob, &SocketJob::signalGetDocs, this, &MainWindow::slotGetDocs);
    connect(_socketJob, &SocketJob::signalGetFavDocs, this, &MainWindow::slotGetFavDocs);
    connect(_socketJob, &SocketJob::signalSendDocInfo, this, &MainWindow::slotSendDocInfo);
    connect(_socketJob, &SocketJob::signalSendDocToClient, this, &MainWindow::slotSendDocToClient);

    // Настраиваем интерфейс
    ReloadStyle(ui->page_docs,      "inside");
    ReloadStyle(ui->page_favDocs,   "inside");
    ReloadStyle(ui->sa_contents,    "inside");
    ReloadStyle(ui->sa_favContents, "inside");

    // Настраиваем кнопки-картинки
    ConfigureUserToolButtons();
    _iconFavDoc      = new QIcon(_iconsPath + "star.png");       // Для "Избранное"
    _iconDownloadDoc = new QIcon(_iconsPath + "download.png");   // Для "Скачать"

    // Отправляем запрос на список документов и избранных документов
    _socketJob->SendSocketQuery(Commands::getDocs);
    _socketJob->SendSocketQuery(Commands::getFavDocs);
}

void MainWindow::SetupSettingsForAdmin()
{
    // Инициализируем формы для админа
    InitializeFormsForAdmin();

    // Коннектим сигналы
    connect(_socketJob, &SocketJob::signalGetRolesAndLevels, _uiProfileCreate, &ProfileCreate::slotRolesAndLevels);
    connect(_socketJob, &SocketJob::signalGetRolesAndLevels, _uiProfileEdit, &ProfileEdit::slotRolesAndLevels);

    // Настраиваем кнопки-картинки
    ConfigureAdminToolButtons();

    // Отправляем запрос на список ролей и уровней
    _socketJob->SendSocketQuery(Commands::getRolesAndLevels);
}

void MainWindow::SetupSettingsForModerator()
{
    // Инициализируем формы для модератора
    InitializeFormsForModer();

    // Коннектим сигналы-слоты
    connect(_socketJob, &SocketJob::signalGetRolesAndLevels, _uiDocsEdit, &DocsEdit::slotRolesAndLevels);

    // Настраиваем кнопки-картинки
    ConfigureModerToolButtons();

    // Отправляем запрос на список ролей и уровней
    _socketJob->SendSocketQuery(Commands::getRolesAndLevels);
}

void MainWindow::ConfigureUserToolButtons()
{
    ConfigureToolButton(ui->b_docs,        _iconsPath + "docs.png",   "Документы");
    ConfigureToolButton(ui->b_favourites,  _iconsPath + "star.png",   "Избранное");
    ConfigureToolButton(ui->b_upload,      _iconsPath + "upload.png", "Загрузить");
    ConfigureToolButton(ui->b_profileUser, _iconsPath + "user.png",   "Профиль");
}

void MainWindow::ConfigureAdminToolButtons()
{
    ConfigureToolButton(ui->b_createUser,   _iconsPath + "user_add.png",   "Новый\nаккаунт",     QSize(60,40));
    ConfigureToolButton(ui->b_editUser,     _iconsPath + "user_edit.png",  "Редакция\nаккаунта", QSize(60,40));
    ConfigureToolButton(ui->b_checkProfile, _iconsPath + "user_check.png", "Просмотр\nпрофилей", QSize(60,40));
    ConfigureToolButton(ui->b_profileAdmin, _iconsPath + "user.png",       "Профиль");
}

void MainWindow::ConfigureModerToolButtons()
{
    ConfigureToolButton(ui->b_docsInfoEdit, _iconsPath + "doc_edit.png",   "Документы");
    ConfigureToolButton(ui->b_statistics,   _iconsPath + "statistics.png", "Статистика");
    ConfigureToolButton(ui->b_profileModer, _iconsPath + "user.png",        "Профиль");
}

void MainWindow::InitializeFormsForUser()
{
    // Инициализируем формы и добавляем их в stackedWidget
    _layoutDocs    = new QVBoxLayout();     // Лейаут для документов
    _layoutFavDocs = new QVBoxLayout();     // Лейаут для избранных документов

    _uiSearcherDocs = new Searcher(_socketJob, Commands::getDocs);      // Поисковик документов
    ui->lay_searcherDocs->addWidget(_uiSearcherDocs);                   // Добавляем его в layout

    _uiSearcherFavDocs = new Searcher(_socketJob, Commands::getFavDocs, this); // Поисковик избранных документов
    ui->lay_searcherFavDocs->addWidget(_uiSearcherFavDocs);             // Добавляем его в layout

    _uiUpload = new Upload(_socketJob);     // Загрузка документа
    ui->stackedWidget->addWidget(_uiUpload);
}

void MainWindow::InitializeFormsForAdmin()
{
    // Инициализируем формы и добавляем их в stackedWidget
    _uiProfileCreate = new ProfileCreate(_socketJob); // Добавление аккаунта
    ui->stackedWidget->addWidget(_uiProfileCreate);

    _uiProfileEdit = new ProfileEdit(_socketJob);     // Редактирование аккаунта
    ui->stackedWidget->addWidget(_uiProfileEdit);

    _uiProfileCheck = new ProfileCheck(_socketJob);   // Просмотр профилей
    ui->stackedWidget->addWidget(_uiProfileCheck);
}

void MainWindow::InitializeFormsForModer()
{
    // Инициализируем формы и добавляем их в stackedWidget
    _uiDocsEdit = new DocsEdit(_socketJob, _iconsPath); // Редакция документов
    ui->stackedWidget->addWidget(_uiDocsEdit);

    _uiStatistics = new Statistics(_socketJob);  // Статистика
    ui->stackedWidget->addWidget(_uiStatistics);
}

void MainWindow::QuitForUser()
{
    // Дисконнектим сигналы-слоты
    disconnect(_socketJob, &SocketJob::signalGetDocs, this, &MainWindow::slotGetDocs);
    disconnect(_socketJob, &SocketJob::signalGetFavDocs, this, &MainWindow::slotGetFavDocs);
    disconnect(_socketJob, &SocketJob::signalSendDocInfo, this, &MainWindow::slotSendDocInfo);
    disconnect(_socketJob, &SocketJob::signalSendDocToClient, this, &MainWindow::slotSendDocToClient);

    // Освобождаем выделенную память и вытаскиваем формы из стакед виджета
    while (QLayoutItem* item = _layoutDocs->takeAt(0))  // Очищаем виджет лейаута документов
    {
        item->widget()->deleteLater();
        delete item;
    }
    _layoutDocs->deleteLater();

    while (QLayoutItem* item = _layoutFavDocs->takeAt(0)) // Очищаем виджет лейаута избранных документов
    {
        item->widget()->deleteLater();
        delete item;
    }
    _layoutFavDocs->deleteLater();

    ui->lay_searcherDocs->takeAt(0)->widget()->deleteLater();    // Поисковик документов
    ui->lay_searcherFavDocs->takeAt(0)->widget()->deleteLater(); // Поисковик избранных документов

    ui->stackedWidget->removeWidget(_uiUpload); // Загрузка документа
    _uiUpload->deleteLater();
}

void MainWindow::QuitForAdmin()
{
    // Дисконнектим сигналы
    disconnect(_socketJob, &SocketJob::signalGetRolesAndLevels, _uiProfileCreate, &ProfileCreate::slotRolesAndLevels);
    disconnect(_socketJob, &SocketJob::signalGetRolesAndLevels, _uiProfileEdit, &ProfileEdit::slotRolesAndLevels);

    // Освобождаем выделенную память и вытаскиваем формы из стакед виджета
    ui->stackedWidget->removeWidget(_uiProfileCreate); // Добавление аккаунта
    ui->stackedWidget->removeWidget(_uiProfileEdit);   // Редактирование аккаунта
    ui->stackedWidget->removeWidget(_uiProfileCheck);  // Просмотр профилей
    _uiProfileCreate->deleteLater();
    _uiProfileEdit->deleteLater();
    _uiProfileCheck->deleteLater();
}

void MainWindow::QuitForModer()
{
    // Дисконнектим сигналы-слоты
    disconnect(_socketJob, &SocketJob::signalGetRolesAndLevels, _uiDocsEdit, &DocsEdit::slotRolesAndLevels);

    // Освобождаем выделенную память и вытаскиваем формы из стакед виджета
    ui->stackedWidget->removeWidget(_uiDocsEdit);      // Редакция документов
    ui->stackedWidget->removeWidget(_uiStatistics);    // Статистика
    _uiDocsEdit->deleteLater();
    _uiStatistics->deleteLater();
}

void MainWindow::slotGetDocs(QJsonObject *obj)
{
    // Очищаем виджет лейаута документов
    while (QLayoutItem* item = _layoutDocs->takeAt(0))
    {
        item->widget()->deleteLater();
        delete item;
    }

    // Объект - это массив документов
    QJsonArray docsArr = obj->value("docs").toArray();

    // Создаём шапку
    Document templateDoc{-1, "-1", -1, false};
    Docs* ui_doc = new Docs(_socketJob, &templateDoc, true, _iconFavDoc, _iconDownloadDoc);

    // Добавляем её в layout
    _layoutDocs->addWidget(ui_doc);

    // Разбиваем массив документов на документы и работаем с ними
    for (int i = 0; i < docsArr.size(); ++i)
    {
        QJsonObject doc(docsArr[i].toObject());

        int id_doc(doc.value("id_doc").toInt());
        QString name(doc.value("name").toString());
        int level(doc.value("level").toInt());
        bool favorite(doc.value("favorite").toBool());

        // Создаём виджет документа и заполняем его шаблоном
        Document templateDoc{id_doc, name, level, favorite};
        Docs* ui_document = new Docs(_socketJob, &templateDoc, false, _iconFavDoc, _iconDownloadDoc);

        // Добавляем его в layout для stackedWidget'а
        _layoutDocs->addWidget(ui_document);

        // Коннектим сигналы
        connect(ui_document, &Docs::signalUpdateDocs, this, &MainWindow::slotUpdateListsDocs);
    }

    // Добавляем стретч для компоновки виджетов и добавляем layout в scrollArea
    _layoutDocs->addStretch();
    ui->sa_contents->setLayout(_layoutDocs);

    // Оповещаем пользователя
    ui->statusbar->showMessage("Документы обновлены!", _messageTime);
}

void MainWindow::slotGetFavDocs(QJsonObject *obj)
{
    // Очищаем виджет лейаута избранных документов
    while (QLayoutItem* item = _layoutFavDocs->takeAt(0))
    {
        item->widget()->deleteLater();
        delete item;
    }

    // Объект - это массив документов
    QJsonArray docsArr = obj->value("docs").toArray();

    // Создаём шапку
    Document templateDoc{-1, "-1", -1, false};
    Docs* ui_doc = new Docs(_socketJob, &templateDoc, true, _iconFavDoc, _iconDownloadDoc);

    // Добавляем её в layout
    _layoutFavDocs->addWidget(ui_doc);

    // Разбиваем массив документов на документы и работаем с ними
    for (int i = 0; i < docsArr.size(); ++i)
    {
        QJsonObject doc(docsArr[i].toObject());

        int id_doc(doc.value("id_doc").toInt());
        QString name(doc.value("name").toString());
        int level(doc.value("level").toInt());
        bool favorite(doc.value("favorite").toBool());

        // Создаём виджет документа и заполняем его шаблоном
        Document templateDoc{id_doc, name, level, favorite};
        Docs* ui_document = new Docs(_socketJob, &templateDoc, false, _iconFavDoc, _iconDownloadDoc);

        // Добавляем его в layout для stackedWidget'а
        _layoutFavDocs->addWidget(ui_document);

        // Коннектим сигналы
        connect(ui_document, &Docs::signalUpdateDocs, this, &MainWindow::slotUpdateListsDocs);
    }

    // Добавляем стретч для компоновки виджетов и добавляем layout в scrollArea
    _layoutFavDocs->addStretch();
    ui->sa_favContents->setLayout(_layoutFavDocs);

    // Оповещаем пользователя
    ui->statusbar->showMessage("Документы обновлены!", _messageTime);
}

void MainWindow::slotSendDocInfo(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if (success)
    {
        // Получаем имя файла
        QString docName(obj->value("name").toString());

        // Сохраняем файл
        QString fileName(QFileDialog::getSaveFileName(this, "Сохранить документ как", docName));

        // Если место для сохранения файла выбрано
        if (fileName != "")
        {
            // Запоминаем место для сохранения файла
            _socketJob->SetFileName(fileName);

            // Достаём id документа
            int docId = _socketJob->GetLastDocId();

            // Формируем json объект
            QJsonObject object;
            object.insert("id_doc", docId);

            // Отправляем объект на сервер
            _socketJob->SendSocketQuery(Commands::sendDocToClient, &object);
        }
        else
            ui->statusbar->showMessage("Загрузка файла отменена", _messageTime);
    }
    else
        ui->statusbar->showMessage(obj->value("error").toString(), _messageTime);
}

void MainWindow::slotSendDocToClient(QJsonObject *obj)
{
    bool success = obj->value("success").toBool();

    if (success)
    {
        // Объект - это json с документом внутри в виде QString(QByteArray.toBase64())
        QString documentFromString(obj->value("document").toString());
        QByteArray documentFromByte(documentFromString.toUtf8());
        documentFromByte = documentFromByte.fromBase64(documentFromByte);

        // Достаём место сохранения файла
        QString fileName(_socketJob->GetFileName());

        // Сохраняем файл
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(documentFromByte);
            file.close();
        }

        // Оповещаем пользователя
        ui->statusbar->showMessage("Файл скачан!", _messageTime);
    }
    else
        ui->statusbar->showMessage(obj->value("error").toString(), _messageTime);
}

void MainWindow::slotUpdateListsDocs()
{
    // Теперь, при нажатии на кнопки "Документы" и "Избранное"
    // список документов в них обновится
    _isNeedUpdateListsDocs = true;
}

void MainWindow::on_b_docs_clicked()
{
    // При необходимости обновляем список документов
    if(_isNeedUpdateListsDocs)
    {
        // Отправляем запрос на список документов
        _socketJob->SendSocketQuery(Commands::getDocs);
        _isNeedUpdateListsDocs = false;
    }

    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(ui->page_docs);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_docs);

    // Меняем активный путь
    SetNewPathForLabel("Документы");
}

void MainWindow::on_b_favourites_clicked()
{
    // При необходимости обновляем список документов
    if(_isNeedUpdateListsDocs)
    {
        // Отправляем запрос на список избранных документов
        _socketJob->SendSocketQuery(Commands::getFavDocs);
        _isNeedUpdateListsDocs = false;
    }

    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(ui->page_favDocs);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_favourites);

    // Меняем активный путь
    SetNewPathForLabel("Избранные документы");
}

void MainWindow::on_b_upload_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiUpload);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_upload);

    // Меняем активный путь
    SetNewPathForLabel("Загрузить");
}

void MainWindow::on_b_profileUser_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiUser);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_profileUser);

    // Меняем активный путь
    SetNewPathForLabel("Профиль пользователя");
}


void MainWindow::SetDefaultPathForLabel()
{
    _activeLabelPath = _defaultLabelPath;
    ui->l_path->setText(_activeLabelPath);
}

void MainWindow::SetNewPathForLabel(const char *path)
{
    _activeLabelPath = _defaultLabelPath + " / " + QString(path);
    ui->l_path->setText(_activeLabelPath);
}

void MainWindow::ConfigureToolButton(QToolButton *btn, const QString &iconPath, const QString &text, QSize size)
{
    QIcon icon(iconPath);
    btn->setIconSize(size);
    btn->setIcon(icon);
    btn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    btn->setText(text);
}

void MainWindow::SwapButtonPalette(QAbstractButton* b_active)
{
    if (_activeButton != nullptr)
    {
        _activeButton->setChecked(false);
        b_active->setChecked(true);
    }
    else
       b_active->setChecked(true);

    // Не забываем "переключать" активную кнопку
    _activeButton = b_active;
}

void MainWindow::ReloadStyle(QWidget *widget, const QString& name)
{
    if (name != "")
        widget->setObjectName(name);

    style()->unpolish(widget);
    style()->polish(widget);
}

void MainWindow::ShowLayout(QLayout *lay)
{
    for(int i = 0; i < lay->count(); ++i)
    {
        QWidget* widget = lay->itemAt(i)->widget();
        if (widget != nullptr)
            widget->show();
    }
}

void MainWindow::HideLayout(QLayout *lay)
{
    for(int i = 0; i < lay->count(); ++i)
    {
        QWidget* widget = lay->itemAt(i)->widget();
        if (widget != nullptr)
            widget->hide();
    }
}

void MainWindow::HideAllLayouts()
{
    HideLayout(ui->lay_leftMenuUser);
    HideLayout(ui->lay_leftMenuAdmin);
    HideLayout(ui->lay_leftMenuModerator);
    HideLayout(ui->lay_topMenu);
    HideLayout(ui->lay_topMenuExtra);
}

void MainWindow::HideSpacer(QSpacerItem *spacer)
{
    // Настройки для скрытия спейсеров: 0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed
    if (spacer != nullptr)
        spacer->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void MainWindow::HideAllSpacers()
{
    HideSpacer(ui->spacer_top);
    HideSpacer(ui->spacer_left_top_admin);
    HideSpacer(ui->spacer_left_down_admin);
    HideSpacer(ui->spacer_left_top_user);
    HideSpacer(ui->spacer_left_down_user);
    HideSpacer(ui->spacer_left_top_moder);
    HideSpacer(ui->spacer_left_down_moder);
}

void MainWindow::ShowMainSpacers()
{
    // Настройки для отображения спейсеров
    // горизонтальные: 1,1, QSizePolicy::Expanding, QSizePolicy::Fixed
    // вертикальные:   1,1, QSizePolicy::Fixed, QSizePolicy::Expanding
    ui->spacer_top->changeSize(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void MainWindow::ShowRoleSpacers(QSpacerItem *top, QSpacerItem *down)
{
    if (top != nullptr)
        top->changeSize(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);

    if (down != nullptr)
        down->changeSize(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void MainWindow::on_b_profileAdmin_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiUser);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_profileAdmin);

    // Меняем активный путь
    SetNewPathForLabel("Профиль пользователя");
}

void MainWindow::on_b_createUser_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiProfileCreate);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_createUser);

    // Меняем активный путь
    SetNewPathForLabel("Создание аккаунта");
}

void MainWindow::on_b_editUser_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiProfileEdit);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_editUser);

    // Меняем активный путь
    SetNewPathForLabel("Редакция аккаунта");
}


void MainWindow::on_b_quit_clicked()
{
    // Очищаем общий профиль пользователя и убираем его из stacked widget
    ui->stackedWidget->removeWidget(_uiUser);
    _uiUser->deleteLater();

    // Настраиваем интерфейс
    HideAllLayouts();
    HideAllSpacers();
    ReloadStyle(ui->stackedWidget, "likewidget");

    // В зависимости от роли, очищаем и высвобождаем необходимое
    switch (_socketJob->GetUserRole())
    {
        default:
        case 2: QuitForUser();  break;
        case 1: QuitForAdmin(); break;
        case 3: QuitForModer(); break;
    }

    // Показываем форму логина в стаке виджетов
    ui->stackedWidget->setCurrentWidget(_uiLogin);

    // Отправляем серверу оповещение о выходе из аккаунта
    _socketJob->SendSocketQuery(Commands::quit);
}


void MainWindow::on_b_checkProfile_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiProfileCheck);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_checkProfile);

    // Меняем активный путь
    SetNewPathForLabel("Просмотр профилей");
}


void MainWindow::on_b_docsInfoEdit_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiDocsEdit);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_docsInfoEdit);

    // Меняем активный путь
    SetNewPathForLabel("Редакция информации о документах");
}

void MainWindow::on_b_statistics_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiStatistics);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_statistics);

    // Меняем активный путь
    SetNewPathForLabel("Статистика");
}

void MainWindow::on_b_profileModer_clicked()
{
    // Устанавливаем виджет профиля
    ui->stackedWidget->setCurrentWidget(_uiUser);

    // Меняем паллету кнопки
    SwapButtonPalette(ui->b_profileModer);

    // Меняем активный путь
    SetNewPathForLabel("Профиль пользователя");
}

