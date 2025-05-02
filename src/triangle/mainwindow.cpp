#include "mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDateTime>
#include <QScrollBar>

static QLabel* makeValueLabel(const QString& text = "—")
{
    auto* lbl = new QLabel(text);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return lbl;
}

static QLabel* makeKeyLabel(const QString& text)
{
    auto* lbl = new QLabel(text);
    lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl->setObjectName("keyLabel");
    return lbl;
}

static QFrame* makeSeparator()
{
    auto* f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setObjectName("separator");
    return f;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("🛰  Mission Control");
    setMinimumSize(980, 660);
    resize(1100, 720);

    setupUi();
    applyStyles();

    m_workerThread = new QThread(this);
    m_worker       = new RoverWorker("127.0.0.1", 9000);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started,
            m_worker, &RoverWorker::connectToRover);

    connect(m_worker, &RoverWorker::stateUpdated,
            this,     &MainWindow::onStateUpdated,
            Qt::QueuedConnection);

    connect(m_worker, &RoverWorker::connectionChanged,
            this,     &MainWindow::onConnectionChanged,
            Qt::QueuedConnection);

    connect(m_worker, &RoverWorker::logMessage,
            this,     &MainWindow::onLogMessage,
            Qt::QueuedConnection);

    connect(m_btnStart, &QPushButton::clicked, this, [this](){
        QMetaObject::invokeMethod(m_worker, [this](){
            m_worker->sendCommand("START");
        });
    });
    connect(m_btnStop, &QPushButton::clicked, this, [this](){
        QMetaObject::invokeMethod(m_worker, [this](){
            m_worker->sendCommand("STOP");
        });
    });
    connect(m_btnAbort, &QPushButton::clicked, this, [this](){
        QMetaObject::invokeMethod(m_worker, [this](){
            m_worker->sendCommand("ABORT");
        });
        onLogMessage("🔴  ABORT command sent");
    });

    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(600);
    connect(m_blinkTimer, &QTimer::timeout, this, &MainWindow::onBlinkTick);
    m_blinkTimer->start();

    m_workerThread->start();
    onLogMessage("🚀  Mission Control initialised — waiting for rover…");
}

MainWindow::~MainWindow()
{
    m_workerThread->quit();
    m_workerThread->wait(3000);
    delete m_worker;
}


QPushButton* MainWindow::makeButton(const QString& text, const QString& color)
{
    auto* btn = new QPushButton(text);
    btn->setFixedHeight(38);
    btn->setProperty("btnColor", color);
    return btn;
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* topBar   = new QWidget;
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(52);

    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(18, 0, 18, 0);

    auto* titleLbl = new QLabel("🛰  MISSION CONTROL");
    titleLbl->setObjectName("titleLabel");

    m_connDot   = new QLabel("●");
    m_connDot->setObjectName("connDotOff");
    m_connLabel = new QLabel("NO SIGNAL");
    m_connLabel->setObjectName("connLabel");

    auto* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    topLayout->addWidget(titleLbl);
    topLayout->addWidget(spacer);
    topLayout->addWidget(m_connDot);
    topLayout->addWidget(m_connLabel);

    auto* bodyWidget  = new QWidget;
    auto* bodyLayout  = new QHBoxLayout(bodyWidget);
    bodyLayout->setContentsMargins(12, 8, 12, 8);
    bodyLayout->setSpacing(12);

    m_map = new MapWidget;
    m_map->setObjectName("mapPanel");
    bodyLayout->addWidget(m_map, 3);

    auto* rightPanel = new QWidget;
    rightPanel->setObjectName("rightPanel");
    rightPanel->setFixedWidth(280);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(12, 12, 12, 12);
    rightLayout->setSpacing(6);


    auto* teleTitle = new QLabel("TELEMETRY");
    teleTitle->setObjectName("sectionTitle");
    rightLayout->addWidget(teleTitle);
    rightLayout->addWidget(makeSeparator());

    auto* statusRow = new QHBoxLayout;
    statusRow->addWidget(makeKeyLabel("STATUS"));
    m_statusVal = makeValueLabel("IDLE");
    m_statusVal->setObjectName("statusVal");
    statusRow->addWidget(m_statusVal);
    rightLayout->addLayout(statusRow);

    auto* hdgRow = new QHBoxLayout;
    hdgRow->addWidget(makeKeyLabel("HEADING"));
    m_headingVal = makeValueLabel("000.0 °");
    hdgRow->addWidget(m_headingVal);
    rightLayout->addLayout(hdgRow);

    auto* uptRow = new QHBoxLayout;
    uptRow->addWidget(makeKeyLabel("UPTIME"));
    m_uptimeVal = makeValueLabel("00:00:00");
    uptRow->addWidget(m_uptimeVal);
    rightLayout->addLayout(uptRow);

    rightLayout->addSpacing(4);
    rightLayout->addWidget(makeKeyLabel("BATTERY"));
    m_batteryBar = new QProgressBar;
    m_batteryBar->setObjectName("batteryBar");
    m_batteryBar->setRange(0, 100);
    m_batteryBar->setValue(100);
    m_batteryBar->setFixedHeight(16);
    m_batteryBar->setTextVisible(true);
    m_batteryBar->setFormat("%v %");
    rightLayout->addWidget(m_batteryBar);

    rightLayout->addSpacing(4);
    rightLayout->addWidget(makeKeyLabel("SPEED"));
    m_speedBar = new QProgressBar;
    m_speedBar->setObjectName("speedBar");
    m_speedBar->setRange(0, 80);
    m_speedBar->setValue(0);
    m_speedBar->setFixedHeight(16);
    m_speedBar->setTextVisible(true);
    m_speedBar->setFormat("0.0 m/s");
    rightLayout->addWidget(m_speedBar);


    rightLayout->addSpacing(10);
    auto* logTitle = new QLabel("EVENT LOG");
    logTitle->setObjectName("sectionTitle");
    rightLayout->addWidget(logTitle);
    rightLayout->addWidget(makeSeparator());

    m_eventLog = new QListWidget;
    m_eventLog->setObjectName("eventLog");
    m_eventLog->setSpacing(1);
    rightLayout->addWidget(m_eventLog, 1);

    bodyLayout->addWidget(rightPanel, 0);

    auto* bottomBar    = new QWidget;
    bottomBar->setObjectName("bottomBar");
    bottomBar->setFixedHeight(62);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(18, 8, 18, 8);
    bottomLayout->setSpacing(10);

    m_btnStart = makeButton("▶  START / AUTO", "green");
    m_btnStop  = makeButton("⏸  STOP",         "yellow");
    m_btnAbort = makeButton("🔴  ABORT",        "red");
    m_btnAbort->setObjectName("abortBtn");

    auto* bSpacer = new QWidget;
    bSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    bottomLayout->addWidget(m_btnStart);
    bottomLayout->addWidget(m_btnStop);
    bottomLayout->addWidget(bSpacer);
    bottomLayout->addWidget(m_btnAbort);

    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(topBar);
    rootLayout->addWidget(bodyWidget, 1);
    rootLayout->addWidget(bottomBar);
}


void MainWindow::applyStyles()
{
    qApp->setFont(QFont("Courier New", 10));

    setStyleSheet(R"(

        QMainWindow, QWidget {
            background-color: #0a0c12;
            color: #b0c8b8;
        }


        #topBar {
            background-color: #0d1018;
            border-bottom: 1px solid #1a2830;
        }
        #titleLabel {
            color: #00f5a0;
            font-size: 15px;
            font-weight: bold;
            letter-spacing: 3px;
        }
        #connLabel {
            color: #3a5a48;
            font-size: 10px;
            letter-spacing: 2px;
        }
        #connDotOff  { color: #2a3a30; font-size: 14px; }
        #connDotOn   { color: #00f5a0; font-size: 14px; }
        #connDotBlink{ color: #007a50; font-size: 14px; }


        #rightPanel {
            background-color: #0d1018;
            border: 1px solid #182230;
            border-radius: 4px;
        }
        #sectionTitle {
            color: #00c880;
            font-size: 10px;
            letter-spacing: 3px;
            padding-top: 4px;
        }
        #separator {
            color: #182230;
            background-color: #182230;
        }
        #keyLabel {
            color: #3a5a48;
            font-size: 10px;
            letter-spacing: 1px;
        }
        QLabel {
            color: #78a888;
            font-size: 11px;
        }
        #statusVal   { color: #00f5a0; font-weight: bold; letter-spacing: 1px; }


        QProgressBar {
            border: 1px solid #182230;
            border-radius: 2px;
            background: #0a0f14;
            text-align: right;
            padding-right: 4px;
            color: #3a5a48;
            font-size: 9px;
        }
        #batteryBar::chunk { background-color: #00c880; }
        #speedBar::chunk   { background-color: #00a0d0; }


        #eventLog {
            background-color: #080b10;
            border: 1px solid #182230;
            border-radius: 2px;
            font-size: 9px;
            color: #507060;
        }
        #eventLog::item { padding: 2px 4px; border-bottom: 1px solid #10181a; }
        #eventLog::item:selected { background: #0d2018; color: #00f5a0; }


        #mapPanel {
            border: 1px solid #182230;
            border-radius: 4px;
        }


        #bottomBar {
            background-color: #0d1018;
            border-top: 1px solid #1a2830;
        }


        QPushButton {
            background-color: #0f1820;
            color: #78a888;
            border: 1px solid #1e3030;
            border-radius: 3px;
            padding: 0 18px;
            font-size: 11px;
            letter-spacing: 1px;
            min-width: 130px;
        }
        QPushButton:hover  { background-color: #162530; color: #b0d8b8; }
        QPushButton:pressed{ background-color: #0a1218; }

        #abortBtn {
            color: #e05050;
            border-color: #501818;
            background-color: #180c0c;
            min-width: 130px;
        }
        #abortBtn:hover  { background-color: #280e0e; color: #ff6060; border-color: #802020; }
        #abortBtn:pressed{ background-color: #100808; }


        QScrollBar:vertical {
            background: #080b10;
            width: 6px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #1e3030;
            border-radius: 3px;
            min-height: 20px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }
    )");
}


void MainWindow::onStateUpdated(const RoverState& state)
{
    m_map->updateState(state);

    m_headingVal->setText(QString("%1 °").arg(state.heading_deg, 6, 'f', 1));
    m_uptimeVal->setText(formatUptime(state.uptime_sec));
    m_statusVal->setText(state.status.toUpper());

    m_batteryBar->setValue(int(state.battery));
    m_batteryBar->setFormat(QString("%1 %").arg(state.battery, 0, 'f', 1));
    QString battColour = (state.battery > 50) ? "#00c880"
                       : (state.battery > 20) ? "#e0a020"
                                              : "#e03030";
    m_batteryBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(battColour));

    int spd10 = int(state.speed * 10.0);
    m_speedBar->setValue(qBound(0, spd10, 80));
    m_speedBar->setFormat(QString("%1 m/s").arg(state.speed, 0, 'f', 1));
}

void MainWindow::onConnectionChanged(bool connected)
{
    m_connected = connected;
    m_connLabel->setText(connected ? "CONNECTED" : "NO SIGNAL");
    m_connLabel->setStyleSheet(connected ? "color:#00c880;letter-spacing:2px;"
                                         : "color:#3a5a48;letter-spacing:2px;");
}

void MainWindow::onLogMessage(const QString& msg)
{
    QString ts  = QDateTime::currentDateTime().toString("hh:mm:ss");
    auto*   item = new QListWidgetItem(ts + "  " + msg);

    if (msg.contains("⚠") || msg.contains("Obstacle"))
        item->setForeground(QColor("#d09020"));
    else if (msg.contains("✅") || msg.contains("Connected") || msg.contains("Link"))
        item->setForeground(QColor("#00c880"));
    else if (msg.contains("❌") || msg.contains("ABORT") || msg.contains("lost"))
        item->setForeground(QColor("#e04040"));
    else
        item->setForeground(QColor("#507060"));

    m_eventLog->addItem(item);
    m_eventLog->scrollToBottom();

    while (m_eventLog->count() > 200)
        delete m_eventLog->takeItem(0);
}

void MainWindow::onBlinkTick()
{
    m_blinkState = !m_blinkState;
    if (m_connected) {
        m_connDot->setObjectName(m_blinkState ? "connDotOn" : "connDotBlink");
    } else {
        m_connDot->setObjectName("connDotOff");
    }
    m_connDot->style()->unpolish(m_connDot);
    m_connDot->style()->polish(m_connDot);
}


QString MainWindow::formatUptime(int secs)
{
    int h = secs / 3600;
    int m = (secs % 3600) / 60;
    int s = secs % 60;
    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}
