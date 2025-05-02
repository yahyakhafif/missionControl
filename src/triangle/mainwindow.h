#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include "mapwidget.h"
#include "roverworker.h"
#include "roverstate.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onStateUpdated(const RoverState& state);
    void onConnectionChanged(bool connected);
    void onLogMessage(const QString& msg);
    void onBlinkTick();

private:
    void setupUi();
    void applyStyles();
    QString formatUptime(int secs);
    QPushButton* makeButton(const QString& text, const QString& color);

    MapWidget*    m_map;

    QLabel*       m_connDot;
    QLabel*       m_connLabel;
    QLabel*       m_statusVal;
    QLabel*       m_headingVal;
    QLabel*       m_uptimeVal;
    QProgressBar* m_batteryBar;
    QProgressBar* m_speedBar;

    QListWidget*  m_eventLog;

    QPushButton*  m_btnStart;
    QPushButton*  m_btnStop;
    QPushButton*  m_btnAbort;

    QThread*      m_workerThread;
    RoverWorker*  m_worker;

    QTimer*       m_blinkTimer;
    bool          m_blinkState   = false;
    bool          m_connected    = false;
};
