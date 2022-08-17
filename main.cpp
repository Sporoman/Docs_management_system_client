#include "mainwindow.h"
#include <QApplication>
#include <QFile>

QString GetStyleSheet(QString path);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Получаем страницу стилей
    QString styleSheet = GetStyleSheet("res/Nikishin.qss");

    // Устанавливам стиль приложения
    app.setStyleSheet(styleSheet);

    MainWindow* _window = new MainWindow();
    _window->show();

    return app.exec();
}

QString GetStyleSheet(QString path)
{
    QFile styleSheetFile(path);
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    styleSheetFile.close();

    return styleSheet;
}
