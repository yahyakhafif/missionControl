#include "mapwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QtMath>

MapWidget::MapWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(420, 420);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void MapWidget::updateState(const RoverState& state)
{
    m_state = state;

    m_displayX += (state.x - m_displayX) * 0.18;
    m_displayY += (state.y - m_displayY) * 0.18;

    m_trail.append(QPointF(m_displayX, m_displayY));
    if (m_trail.size() > MAX_TRAIL)
        m_trail.removeFirst();

    update();
}

QPointF MapWidget::worldToScreen(double wx, double wy) const
{
    const double margin = 24.0;
    double scaleX = (width()  - 2.0 * margin) / 100.0;
    double scaleY = (height() - 2.0 * margin) / 100.0;
    return { margin + wx * scaleX, margin + wy * scaleY };
}

void MapWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), QColor(8, 10, 16));

    QRadialGradient vignette(rect().center(), width() * 0.7);
    vignette.setColorAt(0.0, QColor(0, 0, 0, 0));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 120));
    p.fillRect(rect(), vignette);

    for (int i = 0; i <= 5; i++) {
        double w = i * 20.0;
        QPointF a = worldToScreen(w, 0),  b = worldToScreen(w, 100);
        QPointF c = worldToScreen(0, w),  d = worldToScreen(100, w);
        p.setPen(QPen(QColor(35, 45, 65), 1));
        p.drawLine(a, b);
        p.drawLine(c, d);
    }
    for (int i = 0; i <= 10; i++) {
        double w = i * 10.0;
        QPointF a = worldToScreen(w, 0),  b = worldToScreen(w, 100);
        QPointF c = worldToScreen(0, w),  d = worldToScreen(100, w);
        p.setPen(QPen(QColor(22, 28, 42), 1));
        p.drawLine(a, b);
        p.drawLine(c, d);
    }

    p.setPen(QPen(QColor(0, 0, 0, 20), 1));
    for (int y = 0; y < height(); y += 4)
        p.drawLine(0, y, width(), y);

    if (m_trail.size() > 1) {
        for (int i = 1; i < m_trail.size(); i++) {
            float t     = float(i) / float(m_trail.size());
            int   alpha = int(t * 180);
            int   width = (t > 0.85f) ? 2 : 1;
            p.setPen(QPen(QColor(0, 210, 140, alpha), width));
            QPointF p1 = worldToScreen(m_trail[i-1].x(), m_trail[i-1].y());
            QPointF p2 = worldToScreen(m_trail[i].x(),   m_trail[i].y());
            p.drawLine(p1, p2);
        }
    }

    QPointF rPos = worldToScreen(m_displayX, m_displayY);

    QRadialGradient outerGlow(rPos, 28);
    outerGlow.setColorAt(0.0, QColor(0, 255, 170, 55));
    outerGlow.setColorAt(1.0, QColor(0, 255, 170, 0));
    p.setBrush(outerGlow);
    p.setPen(Qt::NoPen);
    p.drawEllipse(rPos, 28, 28);

    QRadialGradient innerGlow(rPos, 12);
    innerGlow.setColorAt(0.0, QColor(0, 255, 170, 100));
    innerGlow.setColorAt(1.0, QColor(0, 255, 170, 0));
    p.setBrush(innerGlow);
    p.drawEllipse(rPos, 12, 12);

    p.save();
    p.translate(rPos);
    p.rotate(m_state.heading_deg - 90.0);

    QPainterPath tri;
    tri.moveTo(  0, -11);
    tri.lineTo(  7,   8);
    tri.lineTo(  0,   4);
    tri.lineTo( -7,   8);
    tri.closeSubpath();

    p.setBrush(QColor(0, 245, 165));
    p.setPen(QPen(QColor(180, 255, 230, 200), 1.0));
    p.drawPath(tri);

    p.setPen(QPen(QColor(0, 255, 170, 100), 1, Qt::DashLine));
    p.drawLine(0, -11, 0, -24);
    p.restore();

    p.setPen(QColor(0, 150, 100));
    p.setFont(QFont("Courier New", 8));
    p.drawText(QRect(6, height() - 18, width() - 12, 16),
               Qt::AlignLeft,
               QString("X: %1   Y: %2   HDG: %3°")
                   .arg(m_state.x,           6, 'f', 2)
                   .arg(m_state.y,           6, 'f', 2)
                   .arg(m_state.heading_deg, 6, 'f', 1));

    p.setPen(QColor(40, 70, 55));
    p.setFont(QFont("Courier New", 7));
    p.drawText(QRect(0, 4, width() - 6, 14), Qt::AlignRight, "ARENA — 100×100 m");
}
