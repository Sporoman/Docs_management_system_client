#ifndef SOCKETJOB_H
#define SOCKETJOB_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "commands.h"

struct UserInfo;

class SocketJob : public QObject
{
    Q_OBJECT

private:
    QTcpSocket* _socket;
    qint64 _nextBlockSize;

    UserInfo* _user;
    int _lastDocId;
    QString _lastFileName;

public:
    SocketJob();
    ~SocketJob();

    void CreateSocket();
    void SocketConnect(QString ip, qint16 port);

    void SendSocketQuery(const Commands::Command commandNum, const QJsonObject* object);
    void SendSocketQuery(const Commands::Command commandNum);

    void SetLastDocId(int docId);
    int  GetLastDocId();
    int  GetUserRole();
    int  GetUserLevel();
    void SetUserLogin(const QString& login);
    void SetFileName(const QString& fileName);
    const QString &GetUserLogin();
    const QString &GetFileName();

public slots:
    void socketReady();
    void socketDisconnected();
    void socketConnected();

signals:
    void signalSuccessfullConnected();
    void signalDisconnected();
    void signalError(QJsonObject*);
    void signalIdentity(QJsonObject*);
    void signalQuit(QJsonObject*);
    void signalUserInfo(QJsonObject*);
    void signalGetDocs(QJsonObject*);
    void signalGetFavDocs(QJsonObject*);
    void signalAddFavoriteDoc(QJsonObject*);
    void signalDeleteFavoriteDoc(QJsonObject*);
    void signalSendDocToClient(QJsonObject*);
    void signalSendDocInfo(QJsonObject*);
    void signalSendDocFullInfo(QJsonObject*);
    void signalSendDocToServer(QJsonObject*);
    void signalGetRolesAndLevels(QJsonObject*);
    void signalAddNewUser(QJsonObject*);
    void signalSearchUserForEdit(QJsonObject*);
    void signalSearchUserForShow(QJsonObject*);
    void signalEditUser(QJsonObject*);
    void signalEditUserInfo(QJsonObject*);
    void signalSetUserStatus(QJsonObject*);
    void signalEditDocInfo(QJsonObject*);
    void signalDeleteDoc(QJsonObject*);
    void signalGetStatistics(QJsonObject*);

private:
    void SetUserInfo(QJsonObject *obj);
    void ShowObject(const Commands::Command comNum, const QJsonObject& obj);
};

struct UserInfo
{
    int level;
    int role;
    QString login;
};

#endif // SOCKETJOB_H
