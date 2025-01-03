/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PACKAGESMANAGER_H
#define PACKAGESMANAGER_H

#include "result.h"
#include "turnpackagearchitecture.h"

#include <memory>

#include <QObject>
#include <QFuture>
#include <QSharedPointer>

#include <QApt/Backend>
#include <QApt/DebFile>

typedef Result<QString> ConflictResult;

class PackageDependsStatus
{
public:
    static PackageDependsStatus ok();
    static PackageDependsStatus available();
    static PackageDependsStatus _break(const QString &package);

    PackageDependsStatus();
    PackageDependsStatus(const int status, const QString &package);
    PackageDependsStatus operator =(const PackageDependsStatus &other);

    PackageDependsStatus max(const PackageDependsStatus &other);
    PackageDependsStatus maxEq(const PackageDependsStatus &other);
    PackageDependsStatus min(const PackageDependsStatus &other);
    PackageDependsStatus minEq(const PackageDependsStatus &other);

    bool isBreak() const;
    bool isAvailable() const;

public:
    int status;
    QString package;
};

class DebListModel;
class PackagesManager : public QObject
{
    Q_OBJECT

    friend class DebListModel;

public:
    explicit PackagesManager(QObject *parent = 0);

    bool isBackendReady();
    bool isArchError(const int idx);
    const ConflictResult packageConflictStat(const int index);
    const ConflictResult isConflictSatisfy(const QString &arch, QApt::Package *package);
    const ConflictResult isInstalledConflict(const QString &packageName, const QString &packageVersion, const QString &packageArch);
    const ConflictResult isConflictSatisfy(const QString &arch, const QList<QApt::DependencyItem> &conflicts);
    int packageInstallStatus(const int index);
    PackageDependsStatus packageDependsStatus(const int index);
    const QString packageInstalledVersion(const int index);
    const QStringList packageAvailableDepends(const int index);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch, const QList<QApt::DependencyItem> &dependsList);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch, const QApt::DependencyItem &candidateItem);
    const QStringList packageReverseDependsList(const QString &packageName, const QString &sysArch);

    void reset();
    void resetPackageDependsStatus(const int index);
    void removePackage(const int index);
    void appendPackage(std::shared_ptr<QApt::DebFile> debPackage,
                       TurnPackageArchitecture::TurnPackage turnPackage = TurnPackageArchitecture::TurnPackage::None);

    std::shared_ptr<QApt::DebFile> const package(const int index) const { return m_preparedPackages[index]; }
    QSharedPointer<QApt::Backend> const backend() const { return m_backendFuture.result(); }

    QFuture<QSharedPointer<QApt::Backend>> m_backendFuture;

private:
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,const QString &architecture, const QList<QApt::DependencyItem> &depends);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,const QString &architecture, const QApt::DependencyItem &candicate);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,const QString &architecture, const QApt::DependencyInfo &dependencyInfo);
    std::pair<QApt::Package *, bool> packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation = QString());

private:

    QList<std::shared_ptr<QApt::DebFile>> m_preparedPackages;
    QList<TurnPackageArchitecture::TurnPackage> m_preparedPackagesTurnStatus;
    QHash<int, int> m_packageInstallStatus;
    QHash<int, PackageDependsStatus> m_packageDependsStatus;
    QSet<QByteArray> m_appendedPackagesMd5;
};

#endif // PACKAGESMANAGER_H
