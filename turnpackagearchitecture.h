/*
 * 用于实现包架构转换的类
 * 如龙芯旧世界转新世界（loongarch64=>loong64）
*/
#ifndef TURNPACKAGEARCHITECTURE_H
#define TURNPACKAGEARCHITECTURE_H

#include <QString>

class TurnPackageArchitecture
{
public:
    enum TurnPackage {
        Loongarch64ToLoong64,
        None
    };

    TurnPackageArchitecture();
    ~TurnPackageArchitecture();
    void unpackLoongarchToLoong64Shell();
    QString turnLoongarchABI1ToABI2(QString debPath);
    QString createTempDir();
    QString debInstallerTempPath();

private:
    QString m_tempDir;
};

#endif // TURNPACKAGEARCHITECTURE_H
