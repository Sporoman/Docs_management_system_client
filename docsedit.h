#ifndef DOCSEDIT_H
#define DOCSEDIT_H

#include <QWidget>
#include <QVBoxLayout>
#include <socket_job.h>

namespace Ui { class DocsEdit; }

struct ActiveDoc;


class DocsEdit : public QWidget
{
    Q_OBJECT

public:
    explicit DocsEdit(SocketJob* socketJob, const QString& iconPath, QWidget *parent = nullptr);
    ~DocsEdit();

private:
    Ui::DocsEdit *ui;
    SocketJob* _socketJob;
    ActiveDoc* _activeDoc;
    QVBoxLayout* _layoutDocs;
    QIcon* _iconEdit;

public slots:
    void slotRolesAndLevels(QJsonObject*);
    void slotSendDocFullInfo(QJsonObject*);
    void slotEditDocInfo(QJsonObject*);
    void slotDeleteDoc(QJsonObject*);

private:
    bool CheckConditions();
    void BlockUserInfo();
    void UnblockUserInfo();
    void ClearInfoLabel();
    void ReloadStyle(QWidget *widget, const QString& name = "");

private slots:
    void slotGetDocs(QJsonObject*);
    void on_b_search_clicked();
    void on_b_update_clicked();
    void on_b_delete_clicked();
    void on_le_idSearch_textEdited(const QString &arg1);
    void on_le_nameSearch_textEdited(const QString &arg1);
    void on_le_name_textEdited(const QString &arg1);
    void on_cb_levelsSearch_activated(int index);
    void on_cb_levels_activated(int index);
};

struct ActiveDoc
{
    QString name;
    QString level;
};

#endif // DOCSEDIT_H
