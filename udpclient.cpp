#include "udpclient.h"

QHostAddress UdpClient::ipAddress;
void UdpClient::setIpAddress(QHostAddress ip)
{
    if (!m_firstPacketSent) {
            ipAddress = QHostAddress(ip);
        } else {
            qDebug() << "Cannot change IP address after the first packet has been sent!";
        }
}

void UdpClient::sendJsonObject(QJsonObject jsonObject)
{
    if (!m_firstPacketSent) {
        m_firstPacketSent = true;
    }
    // Convert the JSON object to a QByteArray
    QJsonDocument jsonDoc(jsonObject);
    QByteArray datagram = jsonDoc.toJson();

    // Send the datagram to the remote host
    socket->writeDatagram(datagram, ipAddress, port);
}

double UdpClient::localProgressBarValue() const
{
    return m_localProgressBarValue;
}

double UdpClient::remoteProgressBarValue() const
{
    return m_remoteProgressBarValue;
}

void UdpClient::readPendingDatagrams()
{
    // Read all pending datagrams
    while (socket->hasPendingDatagrams())
    {
        // Get the size of the datagram
        qint64 size = socket->pendingDatagramSize();

        // Create a QByteArray to hold the datagram
        QByteArray datagram(size, 0);

        // Read the datagram into the QByteArray
        socket->readDatagram(datagram.data(), size);

        // Convert the QByteArray to a JSON document
        QJsonDocument jsonDoc = QJsonDocument::fromJson(datagram);

        // Check if the JSON document is valid
        if (!jsonDoc.isNull())
        {
            // Get the JSON object from the document
            QJsonObject jsonObject = jsonDoc.object();

            // Check if the JSON object contains a "progress" key
            if (jsonObject.contains("progress"))
            {
                // Get the value of the "progress" key
                int progress = jsonObject.value("progress").toInt();

                // Emit the progressBarValueChanged signal with the progress value
                //m_remotePokemon->
                setRemoteProgressBarValue(progress);
            }
        }
    }
}

void UdpClient::setLocalPokemon(QObject *obj)
{
    m_localPokemon = qobject_cast<UserPokemon*>(obj);
}

void UdpClient::lowerProgressBarRemote(double timeInSeconds) {
    int interval = (int)(timeInSeconds * 1000 / 100);
    int value = 100;
    this->remoteStamina.stop();
    QObject::disconnect(remoteStaminaConnection);
    remoteStaminaConnection = QObject::connect(&remoteStamina, &QTimer::timeout, this, [value, this]() mutable { // Wait for the specified interval
        qDebug() << "Not this";
        if (value < 0) {
            remoteStamina.stop();
        }
        setRemoteProgressBarValue(value--); // Update the progress bar value
    });
    remoteStamina.start(interval);
}

void UdpClient::lowerProgressBarLocal(double timeInSeconds) {
    int interval = (int)(timeInSeconds * 1000 / 100);
    int value = 100;
    this->localStamina.stop();
    QObject::disconnect(localStaminaConnection);
    localStaminaConnection = QObject::connect(&localStamina, &QTimer::timeout, this, [=]() mutable{ // Wait for the specified interval
        if (value < 0) {
            localStamina.stop();
        }
        setLocalProgressBarValue(value--); // Update the progress bar value
    });
    localStamina.start(interval);
}

void UdpClient::setLocalProgressBarValue(double value)
{
    m_localProgressBarValue = value;
    emit localProgressBarValueChanged();
}

void UdpClient::timerEvent(QTimerEvent *event)
{
    auto oldValue = m_remoteProgressBarValue;
    oldValue++;
    if (oldValue > 100)
        oldValue = 0;
    setRemoteProgressBarValue(oldValue);
}

void UdpClient::setRemoteProgressBarValue(double value)
{
    m_remoteProgressBarValue = value;
    emit remoteProgressBarValueChanged();
}