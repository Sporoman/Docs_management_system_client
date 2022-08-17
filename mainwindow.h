#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include "socket_job.h"

#include "login.h"
#include "docs.h"
#include "searcher.h"
#include "upload.h"
#include "userprofile.h"
#include "profilecreate.h"
#include "profileedit.h"
#include "profilecheck.h"
#include "docsedit.h"
#include "statistics.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    SocketJob* _socketJob;

    QAbstractButton* _activeButton;
    QString _defaultLabelPath;   // for ui->l_path
    QString _activeLabelPath;    // for ui->l_path
    QVBoxLayout* _layoutDocs;
    QVBoxLayout* _layoutFavDocs;

    // Общие формы
    Login* _uiLogin;
    UserProfile* _uiUser;

    // Формы для пользователя
    Upload*   _uiUpload;
    Searcher* _uiSearcherDocs;
    Searcher* _uiSearcherFavDocs;

    // Формы для админа
    ProfileCreate* _uiProfileCreate;
    ProfileEdit*   _uiProfileEdit;
    ProfileCheck*  _uiProfileCheck;

    // Формы для модератора
    DocsEdit*   _uiDocsEdit;
    Statistics* _uiStatistics;

    QIcon* _iconFavDoc;
    QIcon* _iconDownloadDoc;
    const QString _iconsPath;

    const int _messageTime;
    bool _checkLeftMenu;
    bool _checkTopMenu;
    bool _isNeedUpdateListsDocs;     // Для обновления списков документов при удалении/добавлении в избранное

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void signalQuit();

private slots:
    void slotEnter();
    void slotConnected();
    void slotDisconnect();
    void slotError(QJsonObject*);

    void slotGetDocs(QJsonObject *obj);
    void slotGetFavDocs(QJsonObject *obj);
    void slotSendDocInfo(QJsonObject *obj);
    void slotSendDocToClient(QJsonObject *obj);
    void slotUpdateListsDocs();

    void on_b_docs_clicked();
    void on_b_favourites_clicked();
    void on_b_upload_clicked();
    void on_b_profileUser_clicked();

    void on_b_profileAdmin_clicked();
    void on_b_createUser_clicked();
    void on_b_editUser_clicked();
    void on_b_quit_clicked();
    void on_b_checkProfile_clicked();
    void on_b_docsInfoEdit_clicked();
    void on_b_statistics_clicked();
    void on_b_profileModer_clicked();

private:
    void SetupSettingsForUser();
    void SetupSettingsForAdmin();
    void SetupSettingsForModerator();
    void ConfigureUserToolButtons();
    void ConfigureAdminToolButtons();
    void ConfigureModerToolButtons();
    void InitializeFormsForUser();
    void InitializeFormsForAdmin();
    void InitializeFormsForModer();
    void QuitForUser();
    void QuitForAdmin();
    void QuitForModer();

    void SetDefaultPathForLabel();
    void SetNewPathForLabel(const char *path);
    void ConfigureToolButton(QToolButton* btn, const QString& iconPath, const QString& text, QSize size = QSize(60,50));
    void SwapButtonPalette(QAbstractButton* b_active);
    void ReloadStyle(QWidget *widget, const QString& name = "");

    void ShowLayout(QLayout *lay);
    void HideLayout(QLayout *lay);
    void HideAllLayouts();
    void HideSpacer(QSpacerItem* spacer);
    void HideAllSpacers();
    void ShowMainSpacers();
    void ShowRoleSpacers(QSpacerItem* top, QSpacerItem* down);
};

#endif // MAINWINDOW_H
