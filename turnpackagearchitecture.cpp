#include "turnpackagearchitecture.h"
#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QDateTime>

TurnPackageArchitecture::TurnPackageArchitecture()
{
    // 创建临时文件
    m_tempDir = createTempDir();
}

// 对象销毁时自动移除临时文件
TurnPackageArchitecture::~TurnPackageArchitecture()
{
    if (m_tempDir == "/tmp") {
        return;
    }
    // 清理临时文件
    QDir dir(m_tempDir);
    qDebug() << "Remove Path: " + m_tempDir;
    dir.removeRecursively();
}

QString TurnPackageArchitecture::debInstallerTempPath()
{
    return m_tempDir;
}

QString TurnPackageArchitecture::createTempDir() {
    QProcess process;
    process.start("mktemp", QStringList() << "--suffix=-gxde-deb-installer" << "-d");
    process.waitForStarted();
    process.waitForFinished(-1);
    QString path = process.readAllStandardOutput().replace("\n", "");
    if (path == "") {
        return "/tmp";
    }
    return path;
}

void TurnPackageArchitecture::unpackLoongarchToLoong64Shell()
{
    QFile readShell(":/loongarch-to-loong64.sh");
    QFile writeShell(m_tempDir + "/loongarch-to-loong64.sh");
    readShell.open(QFile::ReadOnly);
    writeShell.open(QFile::WriteOnly);
    writeShell.write(readShell.readAll());
    readShell.close();
    writeShell.close();
}

void TurnPackageArchitecture::unpackAmd64ToAllShell()
{
    QFile readShell(":/amd64-to-all.sh");
    QFile writeShell(m_tempDir + "/amd64-to-all.sh");
    readShell.open(QFile::ReadOnly);
    writeShell.open(QFile::WriteOnly);
    writeShell.write(readShell.readAll());
    readShell.close();
    writeShell.close();
}

QString TurnPackageArchitecture::turnLoongarchABI1ToABI2(QString debPath)
{
    if (!QFile::exists(m_tempDir + "/loongarch-to-loong64.sh")) {
        unpackLoongarchToLoong64Shell();
    }
    QString newPath = m_tempDir + "/" + QString::number(QDateTime::currentDateTime().toTime_t()) + ".deb";
    QProcess process;
    process.start("bash", QStringList() << m_tempDir + "/loongarch-to-loong64.sh"
                  << debPath << newPath);
    process.waitForStarted();
    process.waitForFinished(-1);
    qDebug() << "Normal: " << process.readAllStandardOutput();
    qDebug() << "Error: " << process.readAllStandardError();
    if (QFile::exists(newPath)) {
        return newPath;
    }
    return "";
}


QString TurnPackageArchitecture::turnAmd64ToAll(QString debPath)
{
    if (!QFile::exists(m_tempDir + "/amd64-to-all.sh")) {
        unpackAmd64ToAllShell();
    }
    QString newPath = m_tempDir + "/" + QString::number(QDateTime::currentDateTime().toTime_t()) + ".deb";
    QProcess process;
    process.start("bash", QStringList() << m_tempDir + "/amd64-to-all.sh"
                                        << debPath << newPath);
    process.waitForStarted();
    process.waitForFinished(-1);
    qDebug() << "Normal: " << process.readAllStandardOutput();
    qDebug() << "Error: " << process.readAllStandardError();
    if (QFile::exists(newPath)) {
        return newPath;
    }
    return "";
}
