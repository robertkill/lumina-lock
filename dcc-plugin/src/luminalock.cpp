// SPDX-License-Identifier: GPL-3.0-or-later
#include "luminalock.h"

#include "dccfactory.h"

#include <DConfig>

#include <QFileInfo>
#include <QVariant>

namespace {
const QString kAppId = QStringLiteral("org.lumina.lock");
const QString kTypeKey = QStringLiteral("wallpaperType");
const QString kImageKey = QStringLiteral("wallpaperPath");
const QString kVideoKey = QStringLiteral("videoPath");
const QString kPosterKey = QStringLiteral("posterPath");
const QString kVideoPoolKey = QStringLiteral("videoPaths");
const QString kPosterAlignXKey = QStringLiteral("posterAlignX");
const QString kPosterAlignYKey = QStringLiteral("posterAlignY");
const QString kClockWeightKey = QStringLiteral("clockWeight");
const QString kDateWeightKey = QStringLiteral("dateWeight");
const QString kClockSizeKey = QStringLiteral("clockFontSize");
const QString kDateSizeKey = QStringLiteral("dateFontSize");

constexpr int kAlignDefault = 50;
} // namespace

Luminalock::Luminalock(QObject *parent)
    : QObject(parent)
    , m_config(Dtk::Core::DConfig::create(kAppId, kAppId, QString(), this))
{
    reload();
}

void Luminalock::reload()
{
    if (!m_config || !m_config->isValid())
        return;
    m_wallpaperType = m_config->value(kTypeKey, QStringLiteral("none")).toString();
    m_wallpaperPath = m_config->value(kImageKey).toString();
    m_videoPath = m_config->value(kVideoKey).toString();
    m_posterPath = m_config->value(kPosterKey).toString();
    m_videoPaths = m_config->value(kVideoPoolKey).toStringList();
    m_posterAlignX = m_config->value(kPosterAlignXKey, kAlignDefault).toInt();
    m_posterAlignY = m_config->value(kPosterAlignYKey, kAlignDefault).toInt();
    m_clockWeight = m_config->value(kClockWeightKey, QStringLiteral("light")).toString();
    m_dateWeight = m_config->value(kDateWeightKey, QStringLiteral("medium")).toString();
    m_clockFontSize = m_config->value(kClockSizeKey, 150).toInt();
    m_dateFontSize = m_config->value(kDateSizeKey, 27).toInt();
    Q_EMIT wallpaperTypeChanged(m_wallpaperType);
    Q_EMIT wallpaperPathChanged(m_wallpaperPath);
    Q_EMIT videoPathChanged(m_videoPath);
    Q_EMIT posterPathChanged(m_posterPath);
    Q_EMIT videoPathsChanged(m_videoPaths);
    Q_EMIT posterAlignXChanged(m_posterAlignX);
    Q_EMIT posterAlignYChanged(m_posterAlignY);
    Q_EMIT clockWeightChanged(m_clockWeight);
    Q_EMIT dateWeightChanged(m_dateWeight);
    Q_EMIT clockFontSizeChanged(m_clockFontSize);
    Q_EMIT dateFontSizeChanged(m_dateFontSize);
}

void Luminalock::setConfigValue(const QString &key, const QVariant &value)
{
    if (m_config)
        m_config->setValue(key, value);
    reload();
}

void Luminalock::setType(const QString &type)
{
    setConfigValue(kTypeKey, type);
}

bool Luminalock::setFile(const QString &kind, const QUrl &url)
{
    if (!m_config || !m_config->isValid() || !url.isLocalFile())
        return false;

    const QFileInfo file(url.toLocalFile());
    if (!file.isAbsolute() || !file.isFile() || !file.isReadable())
        return false;

    QString key;
    if (kind == QLatin1String("static"))
        key = kImageKey;
    else if (kind == QLatin1String("video"))
        key = kVideoKey;
    else if (kind == QLatin1String("poster"))
        key = kPosterKey;
    else
        return false;

    m_config->setValue(key, file.absoluteFilePath());
    if (kind != QLatin1String("poster"))
        m_config->setValue(kTypeKey, kind);
    reload();
    return true;
}

bool Luminalock::addVideo(const QUrl &url)
{
    if (!m_config || !m_config->isValid() || !url.isLocalFile())
        return false;

    const QFileInfo file(url.toLocalFile());
    if (!file.isAbsolute() || !file.isFile() || !file.isReadable())
        return false;

    // Adding switches the wallpaper over to the pool, the same way picking a
    // single video switches it to `video`: the list being built is what the user
    // wants to see. Re-adding an entry that is already in the list is a no-op
    // rather than a duplicate, so the draw cannot be weighted by accident.
    if (!m_videoPaths.contains(file.absoluteFilePath())) {
        QStringList paths = m_videoPaths;
        paths.append(file.absoluteFilePath());
        m_config->setValue(kVideoPoolKey, paths);
        m_config->setValue(kTypeKey, QStringLiteral("video-random"));
    }
    reload();
    return true;
}

void Luminalock::removeVideo(const QString &path)
{
    if (!m_config || !m_config->isValid())
        return;

    QStringList paths = m_videoPaths;
    if (!paths.removeAll(path))
        return;

    m_config->setValue(kVideoPoolKey, paths);
    // An emptied pool has nothing to play; stepping back to the built-in
    // wallpaper beats leaving the lock pointed at a video it can no longer load.
    if (paths.isEmpty() && m_wallpaperType == QLatin1String("video-random"))
        m_config->setValue(kTypeKey, QStringLiteral("none"));
    reload();
}

void Luminalock::setPosterAlignX(int percent)
{
    setConfigValue(kPosterAlignXKey, percent);
}

void Luminalock::setPosterAlignY(int percent)
{
    setConfigValue(kPosterAlignYKey, percent);
}

void Luminalock::setClockWeight(const QString &weight)
{
    setConfigValue(kClockWeightKey, weight);
}

void Luminalock::setDateWeight(const QString &weight)
{
    setConfigValue(kDateWeightKey, weight);
}

void Luminalock::setClockFontSize(int size)
{
    setConfigValue(kClockSizeKey, size);
}

void Luminalock::setDateFontSize(int size)
{
    setConfigValue(kDateSizeKey, size);
}

void Luminalock::resetToDefault()
{
    m_config->setValue(kTypeKey, QStringLiteral("none"));
    m_config->setValue(kImageKey, QString());
    m_config->setValue(kVideoKey, QString());
    m_config->setValue(kPosterKey, QString());
    m_config->setValue(kVideoPoolKey, QStringList());
    m_config->setValue(kPosterAlignXKey, kAlignDefault);
    m_config->setValue(kPosterAlignYKey, kAlignDefault);
    m_config->setValue(kClockWeightKey, QStringLiteral("light"));
    m_config->setValue(kDateWeightKey, QStringLiteral("medium"));
    m_config->setValue(kClockSizeKey, 150);
    m_config->setValue(kDateSizeKey, 27);
    reload();
}

DCC_FACTORY_CLASS(Luminalock)
#include "luminalock.moc"
