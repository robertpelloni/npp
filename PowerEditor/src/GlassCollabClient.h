/**
 * GlassCollabClient.h — Real-time Multi-user Collaboration for BobUI
 */

#ifndef GLASS_COLLAB_CLIENT_H
#define GLASS_COLLAB_CLIENT_H

#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class GlassCollabClient : public QObject {
public:
    explicit GlassCollabClient(QObject* parent = nullptr) : QObject(parent) {
        connect(&m_webSocket, &QWebSocket::connected, this, [this](){
            if (onConnected) onConnected();
        });
        connect(&m_webSocket, &QWebSocket::textMessageReceived, this, [this](const QString& msg){
            QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
            QJsonObject obj = doc.object();
            if (onRemoteUpdate) onRemoteUpdate(obj["docId"].toString(), obj["line"].toInt(), obj["text"].toString());
        });
    }

    void connectToServer(const QString& url) {
        m_webSocket.open(QUrl(url));
    }

    void sendUpdate(const QString& docId, int line, const QString& text) {
        QJsonObject obj;
        obj["docId"] = docId;
        obj["line"] = line;
        obj["text"] = text;
        m_webSocket.sendTextMessage(QJsonDocument(obj).toJson());
    }

    std::function<void()> onConnected;
    std::function<void(const QString&, int, const QString&)> onRemoteUpdate;

private:
    QWebSocket m_webSocket;
};

#endif // GLASS_COLLAB_CLIENT_H
