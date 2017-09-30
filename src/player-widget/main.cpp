/*************************************************************************************
    garlic-player: SMIL Player for Digital Signage
    Copyright (C) 2016 Nikolaos Saghiadinos <ns@smil-control.com>
    This file is part of the garlic-player source code

    This program is free software: you can redistribute it and/or  modify
    it under the terms of the GNU Affero General Public License, version 3,
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************************/

#include "mainwindow.h"
#include "cmdparser.h"

QScopedPointer<QFile>   event_log;

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream out(event_log.data());

    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    switch (type)
    {
        case QtInfoMsg:
            out << "INFO ";
            break;
        case QtDebugMsg:
            out << "DEBUG ";
            break;
        case QtWarningMsg:
            out << "WARNING ";
            break;
        case QtCriticalMsg:
            out << "CRITICAL ";
            break;
        case QtFatalMsg:
            out << "FATAL ";
            abort();
            break;
    }
    out << context.category << " " << msg << endl;
    out.flush();    // Clear the buffered data
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TConfiguration *MyConfiguration = new TConfiguration(new QSettings(QSettings::IniFormat, QSettings::UserScope, "SmilControl", "garlic-player"));
    QDir dir(".");
    int ret = 0;
    MyConfiguration->determineBasePath(dir.absolutePath()); // Run in terminal cause absolute path returns user homedirectory in QtCreator
    MyConfiguration->determineUserAgent();
    MyConfiguration->createDirectories();

    // Set the logging file
    event_log.reset(new QFile(MyConfiguration->getPaths("logs") + "events.log"));
    event_log.data()->open(QFile::Append | QFile::Text);
    QLoggingCategory::setFilterRules("*.debug=true\n");
    qInstallMessageHandler(myMessageHandler);

    QApplication::setApplicationName(MyConfiguration->getAppName());
    QApplication::setApplicationVersion(MyConfiguration->getVersion());
    QApplication::setApplicationDisplayName(MyConfiguration->getAppName());

    TCmdParser MyParser(MyConfiguration);
    MyParser.addOptions();
    MyParser.parse(&app);

    bool is_index = true;
    TScreen    MyScreen(0, QApplication::desktop());
    MainWindow w(MyConfiguration, &MyScreen);

    if (MyConfiguration->getIndexUri() == "")
    {
        if (w.openConfigDialog() == QDialog::Rejected)
            is_index = false;
    }

    if (is_index)
    {
        MyScreen.setActualScreenId(MyParser.getScreenSelect());
        w.show();

        QString val = MyParser.getWindowMode();
        if (val == "fullscreen")
            w.resizeAsNormalFullScreen();
        else if (val == "bigscreen")
            w.resizeAsBigFullScreen();
        else if (val == "windowed")
        {
            w.setMainWindowSize(MyParser.getWindowSize());
            w.resizeAsWindow();
        }

        w.checkForNewSmilIndex();
        ret = app.exec();
    }
    return ret;
}
