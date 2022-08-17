#include "socket_job.h"

#include <QDebug>
#include "commands.h"


SocketJob::SocketJob() : _nextBlockSize(0)
{
    // Инициализируем переменные
    _socket = new QTcpSocket();
    _user   = new UserInfo();

    // Подключаем сигналы-слоты
    connect(_socket, SIGNAL(readyRead()),    this, SLOT(socketReady()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(_socket, SIGNAL(connected()),    this, SLOT(socketConnected()));
}

SocketJob::~SocketJob()
{
    _socket->deleteLater();
    delete _user;
}

void SocketJob::SocketConnect(QString ip, qint16 port)
{
    _socket->connectToHost(ip, port);
}

void SocketJob::socketConnected()
{
    qDebug() << "Connected to server!";

    // Испускаем сигнал успешного подключения для класса login
    emit signalSuccessfullConnected();
}

void SocketJob::socketDisconnected()
{
    qDebug() << "Connection is interrupted..";

    // Если не переподключаемся, то испускаем сигнал дисконнекта для класса mainwindow
    if (!_socket->reset())
    {
        qDebug() << "Socket disconnected";
        emit signalDisconnected();
    }
}

void SocketJob::socketReady()
{
    // Создаём поток данных из сокета
    QDataStream in(_socket);
    in.setVersion(QDataStream::Qt_6_2);
    if(in.status() == QDataStream::Ok)
    {
        while(true)
        {
            // Если это свежий блок - записываем его размер
            if (_nextBlockSize == 0)
            {
                if (_socket->bytesAvailable() < sizeof(qint64))
                    break;

                in >> _nextBlockSize;
            }

            // Если размер блока больше, чем доступных байт, значит сообщение ещё не пришло полностью
            if (_nextBlockSize > _socket->bytesAvailable())
                break;

            // Если всё дошло нормально, то обрабатываем входящий запрос,
            // превращая его в Json документ
            QJsonDocument jDoc;
            in >> jDoc;

            // Считываем команду и объект
            Commands::Command command = static_cast<Commands::Command>(jDoc.object().value("commandNumber").toInt());
            QJsonObject object = jDoc.object().value("args").toObject();

            // По разному обрабатываем log'и когда есть документ или фото
            qDebug() << "\n   Get from server:";
            ShowObject(command, object);

            switch(command)
            {
                case Commands::userInfo:          emit signalUserInfo(&object);            break;
                case Commands::getDocs:           emit signalGetDocs(&object);             break;
                case Commands::getFavDocs:        emit signalGetFavDocs(&object);          break;
                case Commands::addFavoriteDoc:    emit signalAddFavoriteDoc(&object);      break;
                case Commands::deleteFavoriteDoc: emit signalDeleteFavoriteDoc(&object);   break;
                case Commands::sendDocInfo:       emit signalSendDocInfo(&object);         break;
                case Commands::sendDocFullInfo:   emit signalSendDocFullInfo(&object);     break;
                case Commands::sendDocToClient:   emit signalSendDocToClient(&object);     break;
                case Commands::sendDocToServer:   emit signalSendDocToServer(&object);     break;
                case Commands::getRolesAndLevels: emit signalGetRolesAndLevels(&object);   break;
                case Commands::addNewUser:        emit signalAddNewUser(&object);          break;
                case Commands::searchUserForEdit: emit signalSearchUserForEdit(&object);   break;
                case Commands::searchUserForShow: emit signalSearchUserForShow(&object);   break;
                case Commands::editUser:          emit signalEditUser(&object);            break;
                case Commands::editUserInfo:      emit signalEditUserInfo(&object);        break;
                case Commands::setUserStatus:     emit signalSetUserStatus(&object);       break;
                case Commands::editDocInfo:       emit signalEditDocInfo(&object);         break;
                case Commands::deleteDoc:         emit signalDeleteDoc(&object);           break;
                case Commands::getStatistics:     emit signalGetStatistics(&object);       break;

                case Commands::quit:              emit signalQuit(&object);                  break;
                case Commands::identity: SetUserInfo(&object); emit signalIdentity(&object); break;

                default: case Commands::error:    emit signalError(&object); break;
            }

            // Если данные ещё остались - запускаем всё по новой
            if (_socket->bytesAvailable() > 0)
            {
                // Обнуляем размер блока
                _nextBlockSize = 0;
                continue;
            }

            // Не забываем обнулять размер блока
            _nextBlockSize = 0;
            break;
        }
    }
    else
        qDebug() << "DataStream Error";
}

void SocketJob::SendSocketQuery(const Commands::Command commandNum, const QJsonObject* args)
{
    // Формируем объект передачи
    QJsonObject obj;
    obj.insert("commandNumber", commandNum);

    // Если есть аргументы - вставляем их в запрос
    if(!args->empty())
        obj.insert("args", *args);

    QJsonDocument doc(obj);

    // По разному обрабатываем log'и когда есть документ или фото
    qDebug() << "\n   Send to server:";
    ShowObject(commandNum, *args);

    // Создаём поток данных
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);

    // Заполняем поток данных заглушкой quint64 под размер сообщения
    // и соответственно самим сообщением
    out << qint64(0) << doc.toJson();

    // Считаем объём сообщения и пишем его вместо заглушки
    out.device()->seek(0);
    out << qint64(data.size() - sizeof(qint64));

    // Отправляем запрос на сервер
    _socket->write(data);
}

void SocketJob::SendSocketQuery(const Commands::Command commandNum)
{
    QJsonObject obj;
    SendSocketQuery(commandNum, &obj);
}

void SocketJob::SetLastDocId(int docId)
{
    _lastDocId = docId;
}

int SocketJob::GetLastDocId()
{
    return _lastDocId;
}

int SocketJob::GetUserRole()
{
    return _user->role;
}

int SocketJob::GetUserLevel()
{
    return _user->level;
}

void SocketJob::SetUserLogin(const QString& login)
{
    _user->login = login;
}

const QString &SocketJob::GetUserLogin()
{
    return _user->login;
}

void SocketJob::SetFileName(const QString& fileName)
{
    _lastFileName = fileName;
}

const QString &SocketJob::GetFileName()
{
    return _lastFileName;
}

void SocketJob::SetUserInfo(QJsonObject *obj)
{
    _user->level = obj->value("level").toInt();
    _user->role  = obj->value("role").toInt();
}

void SocketJob::ShowObject(const Commands::Command comNum, const QJsonObject &obj)
{
    // По разному обрабатываем log'и когда есть документ или фото
    QJsonObject tempArgs(obj); // Временный объект для вывода

    qDebug() << "_command:" << comNum;
    if (obj.contains("document"))
    {
        tempArgs.remove("document");
        tempArgs.insert("document", true);
        qDebug() << "_object:" << tempArgs;
    }
    else if (obj.contains("photo"))
    {
        tempArgs.remove("photo");
        tempArgs.insert("photo", true);
        qDebug() << "_object:" << tempArgs;
    }
    else if (!obj.isEmpty())
        qDebug() << tempArgs;
}
