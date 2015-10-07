/**
 ******************************************************************************
 * @file       copyapp.cpp
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

#include "copyapp.h"
#include "ui_copyapp.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>

CopyApp::CopyApp(QString origin, QString destination, QString app, int timeToWait, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CopyApp),
    origin(origin),
    destination(destination),
    appToRun(app),
    timeToWait(timeToWait),
    started(false)
{
    if(origin.isEmpty() | destination.isEmpty()) {
        QMessageBox::warning(this, "ERROR", "INVALID COMMAND LINE OPTIONS");
        return;
    }
    qDebug() << origin << destination;
    ui->setupUi(this);
    emit infoMessage(tr("Waiting for application to close"));
    QTimer::singleShot(500, this, SLOT(doUpdate()));
    connect(this, &CopyApp::infoMessage, this, &CopyApp::onNewInfo);
    connect(this, &CopyApp::currentOperation, this, &CopyApp::onNewOperation);
    connect(this, &CopyApp::progress, this, &CopyApp::onProgress);
    started = true;
}

CopyApp::~CopyApp()
{
    delete ui;
}

void CopyApp::doUpdate()
{
    static int partial = 0;
    if(partial == 0) {
        if(!checkPermissions(destination)) {
            startAsSudo();
            QApplication::quit();
            return;
        }
    }
    partial += 500;
    if(timeToWait != 0) {
        ui->progressBar->setValue(partial * 100 / timeToWait);
        if(partial < timeToWait) {
            QTimer::singleShot(500, this, SLOT(doUpdate()));
            return;
        }
    }
    emit currentOperation(tr("Looking for local file list info"));
    QStringList localFileList = processFileList(destination);
    emit currentOperation(tr("Looking for remote file list info"));
    QStringList remoteFileList = processFileList(origin);
    if(localFileList.isEmpty() || remoteFileList.isEmpty())
        return;
    emit currentOperation(tr("Deleting old files..."));
    bool result = true;
    result &= deleteFiles(localFileList, destination);
    emit currentOperation(tr("Copying new files..."));
    result &= copyFiles(remoteFileList, origin, destination);
    QString resultStr;
    if(result)
        resultStr = tr("Update finished successfully. ");
    else
        resultStr = tr("Update finished with errors. ");
    if(!appToRun.isEmpty())
        resultStr = resultStr + tr("Click ok to restart application");
    QMessageBox::information(this, tr("Update finished"), resultStr);
    if(!appToRun.isEmpty()) {
        QProcess::startDetached(QDir(destination).filePath(appToRun));
    }
    QApplication::quit();
}

QStringList CopyApp::processFileList(const QString &path)
{
    QStringList ret;
    QFile file(path + QDir::separator() + "filelist.lst");
    emit infoMessage(tr("Looking for infolist.lst file"));
    if(file.exists()) {
        if(!file.open(QIODevice::ReadOnly)) {
            emit infoMessage(tr("Could not open filelist.lst file"));
            QMessageBox::critical(this, tr("Error"), tr("Could not open a required file! Quiting."));
            QApplication::quit();
            return ret;
        }
        else {
            ret = QString(file.readAll()).split("\n");
            file.close();
        }
    }
    else {
        emit infoMessage(tr("Could not find filelist.lst file"));
        QMessageBox::critical(this, tr("Error"), tr("Could not find a required file! Quiting.") + file.fileName());
        QApplication::quit();
        return ret;
    }
    return ret;
}

bool CopyApp::deleteFiles(const QStringList &fileList, const QString &path)
{
    bool ret = true;
    qint64 totalFilesToProcess = fileList.length();
    qint64 filesProcessed = 0;
    QString tempStr;
    foreach (QString fileName, fileList) {
        if(fileName.isEmpty())
            continue;
        bool success = QFile(QDir(path).filePath(fileName)).remove();
        infoMessage(QString(tr("Deleting %0")).arg(fileName));
        ++filesProcessed;
        emit progress(filesProcessed * 100 / totalFilesToProcess);
        if(!success) {
            tempStr = tempStr + fileName + "\n";
            ret = false;
        }
    }
    if(!tempStr.isEmpty())
        QMessageBox::information(this, tr("Delete failed"), tr("The following files failed to be deleted:\n") + tempStr);
    return ret;
}

bool CopyApp::copyFiles(const QStringList &fileList, const QString &originPath, const QString &destinationPath)
{
    bool ret = true;
    QString tempStr;
    qint64 totalFilesToProcess = fileList.length();
    qint64 filesProcessed = 0;
    foreach (QString fileName, fileList) {
        if(fileName.isEmpty())
            continue;
        QString remotePath = QDir(originPath).filePath(fileName);
        QString localPath = QDir(destinationPath).filePath(fileName);
        bool result = QFile::copy(remotePath, localPath);
        if(!result) {
            tempStr = tempStr + localPath + "\n";
            ret = false;
        }
        infoMessage(QString(tr("Copying %0")).arg(fileName));
        ++filesProcessed;
        emit progress(filesProcessed * 100 / totalFilesToProcess);
    }
    if(!tempStr.isEmpty())
        QMessageBox::information(this, tr("Copy failed"), tr("The following files failed to be copied:\n") + tempStr);
    return ret;
}

bool CopyApp::checkPermissions(QString &path)
{
    QFileInfo fi(path);
    if(fi.isDir()) {
        if(fi.isWritable())
            return true;
        else
            return false;
    }
    else {
        Q_ASSERT(0);
        QApplication::quit();
    }
    return false;
}

#ifdef Q_OS_LINUX
void CopyApp::startAsSudo()
{
    QProcess sudo(this);
    QString sudoApp;
    sudo.start("which", QStringList() << "gksudo");
    sudo.waitForFinished();
    sudoApp = QString(sudo.readAllStandardOutput());
    if(sudoApp.isEmpty()) {
        qDebug() << "empty";
        sudo.start("which", QStringList() << "kdesudo");
        sudo.waitForFinished();
        sudoApp = QString(sudo.readAllStandardOutput());
    }
    if(sudoApp.isEmpty()) {
        QMessageBox::critical(this, tr("Wrong permissions"), tr("The target directory permissions require administrator privileges but neither kdesudo or gksudo was found. Try running the application you want to update as sudo. Quiting now!"));
        QApplication::quit();
    }
    QMessageBox::critical(this, tr("Wrong permissions"), tr("The target directory permissions requires this application to restart with administrator privileges!"));
    QProcess::startDetached(sudoApp.simplified(), QStringList() << QString("%0 %1 %2 %4 -w=0").arg(QApplication::applicationFilePath()).arg(origin).arg(destination).arg(appToRun));
}
bool CopyApp::getStarted() const
{
    return started;
}

#endif

void CopyApp::onNewInfo(QString txt)
{
    ui->infoLabel->setText(txt);
    QApplication::processEvents();
}

void CopyApp::onNewOperation(QString txt)
{
    ui->operationLabel->setText(txt);
    QApplication::processEvents();
}

void CopyApp::onProgress(int value)
{
    ui->progressBar->setValue(value);
}
