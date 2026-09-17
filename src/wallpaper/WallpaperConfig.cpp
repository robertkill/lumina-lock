#include "WallpaperConfig.h"

#include "WallpaperManager.h"

#include <DConfig>

#include <QDebug>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QSet>
#include <QUrl>

namespace {
const QString kAppId = QStringLiteral("org.lumina.lock");
const QString kTypeKey = QStringLiteral("wallpaperType");
const QString kImageKey = QStringLiteral("wallpaperPath");
const QString kVideoKey = QStringLiteral("videoPath");
const QString kPosterKey = QStringLiteral("posterPath");
const QString kVideoPoolKey = QStringLiteral("videoPaths");
const QString kPosterAlignXKey = QStringLiteral("posterAlignX");
const QString kPosterAlignYKey = QStringLiteral("posterAlignY");

// Framing is stored as whole percent — 0 = flush with the left/top edge,
// 100 = flush with the right/bottom edge — so it stays readable both in the
// control center and on the command line. 50 is the centred crop.
constexpr int kAlignDefault = 50;
constexpr int kAlignMin = 0;
constexpr int kAlignMax = 100;

const QString kDefaultWallpaper = QStringLiteral("qrc:/assets/wallpapers/default.jpg");

QUrl localOrNull(const QString &path)
{
    return path.isEmpty() ? QUrl() : QUrl::fromLocalFile(path);
}

// Bounds are enforced here rather than trusted from the config: a hand-edited
// value must not be able to pan the poster off its own image.
qreal alignFactor(int percent)
{
    return qreal(qBound(kAlignMin, percent, kAlignMax)) / kAlignMax;
}
} // namespace

WallpaperConfig::WallpaperConfig(QObject *parent)
    : QObject(parent)
    , m_config(Dtk::Core::DConfig::create(kAppId, kAppId, QString(), this))
{
    if (m_config) {
        connect(m_config, &Dtk::Core::DConfig::valueChanged,
                this, &WallpaperConfig::onValueChanged);
    }
}

QString WallpaperConfig::configuredType() const
{
    return m_config && m_config->isValid()
        ? m_config->value(kTypeKey, QStringLiteral("none")).toString()
        : QStringLiteral("none");
}

QStringList WallpaperConfig::videoPool() const
{
    if (!m_config || !m_config->isValid())
        return {};

    // Entries are re-checked on every draw instead of once at write time: a file
    // can be moved or deleted between two locks, and a stale entry must not cost
    // the lock its wallpaper.
    QStringList pool;
    const QStringList configured = m_config->value(kVideoPoolKey).toStringList();
    pool.reserve(configured.size());
    for (const QString &entry : configured) {
        const QString path = entry.trimmed();
        if (path.isEmpty() || pool.contains(path))
            continue;
        const QFileInfo file(path);
        if (file.isFile() && file.isReadable())
            pool.append(path);
    }
    return pool;
}

void WallpaperConfig::applyPosterAlignment(WallpaperManager &wm)
{
    const int x = m_config ? m_config->value(kPosterAlignXKey, kAlignDefault).toInt()
                           : kAlignDefault;
    const int y = m_config ? m_config->value(kPosterAlignYKey, kAlignDefault).toInt()
                           : kAlignDefault;
    wm.setPosterAlignment(alignFactor(x), alignFactor(y));
}

void WallpaperConfig::applyRandomVideo(WallpaperManager &wm)
{
    const QStringList pool = videoPool();
    if (pool.isEmpty()) {
        // Never configured, or every entry has been deleted since: the built-in
        // wallpaper is the safe answer (and the only one that always renders).
        wm.setStaticImage(QUrl(kDefaultWallpaper));
        return;
    }

    // Random, but never the same video twice in a row: the current one is
    // dropped from the draw rather than re-rolled, so a two-entry pool really
    // does alternate and a larger one never repeats back to back.
    QStringList candidates = pool;
    if (candidates.size() > 1)
        candidates.removeAll(m_lastRandomVideo);

    const QString chosen = candidates.at(QRandomGenerator::global()->bounded(candidates.size()));
    m_lastRandomVideo = chosen;
    // One line per lock: which video this lock drew. Cheap, and it is what makes
    // "a different video every time" checkable from the service journal.
    qInfo().noquote() << "Wallpaper: random draw" << chosen;
    wm.setVideo(QUrl::fromLocalFile(chosen), localOrNull(m_config->value(kPosterKey).toString()));
}

void WallpaperConfig::applyTo(WallpaperManager &wm)
{
    applyPosterAlignment(wm);

    const QString type = configuredType();

    if (type == QLatin1String("video")) {
        const QString video = m_config->value(kVideoKey).toString();
        if (!video.isEmpty()) {
            wm.setVideo(localOrNull(video), localOrNull(m_config->value(kPosterKey).toString()));
            return;
        }
    } else if (type == QLatin1String("video-random")) {
        applyRandomVideo(wm);
        return;
    } else if (type == QLatin1String("static")) {
        const QString image = m_config->value(kImageKey).toString();
        if (!image.isEmpty()) {
            wm.setStaticImage(QUrl::fromLocalFile(image));
            return;
        }
    }

    // No configured wallpaper: built-in default.
    wm.setStaticImage(QUrl(kDefaultWallpaper));
}

void WallpaperConfig::pickForNewLock(WallpaperManager &wm)
{
    if (configuredType() == QLatin1String("video-random"))
        applyRandomVideo(wm);
}

void WallpaperConfig::onValueChanged(const QString &key)
{
    static const QSet<QString> watched{kTypeKey, kImageKey, kVideoKey, kPosterKey,
                                      kVideoPoolKey, kPosterAlignXKey, kPosterAlignYKey};
    if (watched.contains(key))
        emit changed();
}
