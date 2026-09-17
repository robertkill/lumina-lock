#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

/**
 * Owns the *what* of the wallpaper (type + source URLs) but not the *how* of
 * rendering it. Rendering lives in QML (WallpaperHost.qml); this class is the
 * single source of truth so wallpaper content never has to be threaded through
 * the rest of the UI.
 *
 * The abstraction is intentionally small: "static" images and "video" are the
 * only concrete kinds right now, and the boundary is wide enough that a plugin
 * or a future "shader" kind can be added without touching LockScreen.qml.
 */
class WallpaperManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type NOTIFY wallpaperChanged)
    Q_PROPERTY(QUrl source READ source NOTIFY wallpaperChanged)
    Q_PROPERTY(QUrl poster READ poster NOTIFY wallpaperChanged)
    Q_PROPERTY(bool isVideo READ isVideo NOTIFY wallpaperChanged)
    // Framing of the poster: where the crop window sits inside the cover-scaled
    // image, 0 = left/top edge, 1 = right/bottom edge. 0.5 (the default) is the
    // centred crop Image.PreserveAspectCrop would pick on its own.
    Q_PROPERTY(qreal posterAlignX READ posterAlignX NOTIFY posterAlignmentChanged)
    Q_PROPERTY(qreal posterAlignY READ posterAlignY NOTIFY posterAlignmentChanged)

public:
    explicit WallpaperManager(QObject *parent = nullptr);

    QString type() const { return m_type; }
    QUrl source() const { return m_source; }
    QUrl poster() const { return m_poster; }
    bool isVideo() const { return m_type == QLatin1String("video"); }
    qreal posterAlignX() const { return m_posterAlignX; }
    qreal posterAlignY() const { return m_posterAlignY; }

    /** Load from a path; the kind is auto-detected from the extension. */
    Q_INVOKABLE bool loadFile(const QString &path, const QString &posterPath = QString());

    Q_INVOKABLE void setStaticImage(const QUrl &url);
    Q_INVOKABLE void setVideo(const QUrl &url, const QUrl &poster = QUrl());
    Q_INVOKABLE void clear();

    /**
     * Place the poster's crop window. Values are clamped to [0, 1] here rather
     * than trusted from the config: this is the one choke point every source of
     * the setting goes through, and an out-of-range value would pan the poster
     * off its own image.
     */
    void setPosterAlignment(qreal x, qreal y);

signals:
    void wallpaperChanged();
    void posterAlignmentChanged();

private:
    static bool isVideoPath(const QString &path);

    QString m_type = QStringLiteral("none");
    QUrl m_source;
    QUrl m_poster;
    qreal m_posterAlignX = 0.5;
    qreal m_posterAlignY = 0.5;
};
