#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>
#include "roverstate.h"

class MapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapWidget(QWidget* parent = nullptr);
    void updateState(const RoverState& state);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    RoverState      m_state;
    QVector<QPointF> m_trail;
    double          m_displayX = 50.0;
    double          m_displayY = 50.0;

    static constexpr int MAX_TRAIL = 300;

    QPointF worldToScreen(double wx, double wy) const;
};
