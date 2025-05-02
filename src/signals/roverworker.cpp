#include "roverworker.h"
#include <QJsonDocument>
#include <QJsonObject>

RoverWorker::RoverWorker(const QString& host, quint16 port, QObject* parent)
    : QObject(parent), m_host(host), m_port(port)
{
    m_socket         = new QTcpSocket(this);
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3000);

    connect(m_socket, &QTcpSocket::readyRead,    this, &RoverWorker::onReadyRead);
    connect(m_socket, &QTcpSocket::connected,    this, &RoverWorker::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &RoverWorker::onDisconnected);
    connect(m_reconnectTimer, &QTimer::timeout,  this, &RoverWorker::tryReconnect);
}

void RoverWorker::connectToRover()
{
    m_socket->connectToHost(m_host, m_port);
    m_reconnectTimer->start();
}

void RoverWorker::sendCommand(const QString& cmd)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        m_socket->write((cmd + "\n").toUtf8());
}

void RoverWorker::onReadyRead()
{
    m_buffer += m_socket->readAll();

    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer        = m_buffer.mid(idx + 1);
        if (line.isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) continue;

        QJsonObject obj = doc.object();

        RoverState state;
        state.timestamp   = obj["timestamp"].toDouble();
        state.x           = obj["x"].toDouble();
        state.y           = obj["y"].toDouble();
        state.heading_deg = obj["heading_deg"].toDouble();
        state.speed       = obj["speed"].toDouble();
        state.battery     = obj["battery"].toDouble();
        state.uptime_sec  = obj["uptime_sec"].toInt();
        state.status      = obj["status"].toString();
        state.error_code  = obj["error_code"].toInt();
        state.event       = obj["event"].toString();

        emit stateUpdated(state);
        if (!state.event.isEmpty())
            emit logMessage(state.event);
    }
}

void RoverWorker::onConnected()
{
    m_reconnectTimer->stop();
    emit connectionChanged(true);
    emit logMessage("✅  Link established with rover");
}

void RoverWorker::onDisconnected()
{
    emit connectionChanged(false);
    emit logMessage("📡  Signal lost — retrying in 3 s…");
    m_reconnectTimer->start();
}

void RoverWorker::tryReconnect()
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState)
        m_socket->connectToHost(m_host, m_port);
}
