#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "roverstate.h"

class RoverWorker : public QObject {
    Q_OBJECT
public:
    explicit RoverWorker(const QString& host, quint16 port, QObject* parent = nullptr);

public slots:
    void connectToRover();
    void sendCommand(const QString& cmd);

signals:
    void stateUpdated(const RoverState& state);
    void connectionChanged(bool connected);
    void logMessage(const QString& msg);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void tryReconnect();

private:
    QTcpSocket*  m_socket;
    QTimer*      m_reconnectTimer;
    QString      m_host;
    quint16      m_port;
    QByteArray   m_buffer;
};
