/**
 ******************************************************************************
 * @file       main.cpp
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2015
 * @addtogroup [Group]
 * @{
 * @addtogroup
 * @{
 * @brief [Brief]
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "copyapp.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("CopyApp");
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Copy application used by Tau Labs updater");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source directory."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));
    parser.addPositionalArgument("app",  QCoreApplication::translate("main", "The name of the application to run after the copy."));
    parser.addOption(QCommandLineOption("w",  QCoreApplication::translate("main", "time to wait before starting the copying process."), "w", "5000"));

    // Process the actual command line arguments given by the user
    parser.process(app);
    const QStringList args = parser.positionalArguments();
    CopyApp w(args.value(0),args.value(1), args.value(2), parser.value("w").toInt());
    if(!w.getStarted())
        return -1;
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - w.width()) / 2;
    int y = (screenGeometry.height() - w.height()) / 2;
    w.move(x, y);
    w.show();
    return app.exec();
}
