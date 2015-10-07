/**
 ******************************************************************************
 * @file       copyapp.h
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2015
 * @addtogroup [Group]
 * @{
 * @addtogroup CopyApp
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

#ifndef COPYAPP_H
#define COPYAPP_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class CopyApp;
}

class CopyApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit CopyApp(QString origin, QString destination, QString app = QString(), int timeToWait = 5000, QWidget *parent = 0);
    ~CopyApp();

    bool getStarted() const;

private slots:
    void doUpdate();
    void onNewInfo(QString txt);
    void onNewOperation(QString txt);
    void onProgress(int value);
private:
    Ui::CopyApp *ui;
    QString origin;
    QString destination;
    int filesProcessed;
    int totalFilesToProcess;
    QString appToRun;
    int timeToWait;
    QStringList processFileList(const QString &path);
    bool deleteFiles(const QStringList &fileList, const QString &path);
    bool copyFiles(const QStringList &fileList, const QString &originPath, const QString &destinationPath);
    bool checkPermissions(QString &path);
    void startAsSudo();
    bool started;
signals:
    void currentOperation(QString);
    void infoMessage(QString);
    void progress(int);
};

#endif // COPYAPP_H
