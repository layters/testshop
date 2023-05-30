#include "daemon_manager.hpp"

#include <iostream>
#include <chrono>
#include <thread>

#include <QtConcurrent/QtConcurrent>
#include <QTcpSocket>

#include "../neroshop_config.hpp"
#include "../core/protocol/transport/client.hpp"

neroshop::DaemonManager::DaemonManager(QObject *parent)
    : QObject{parent}, m_daemonRunning(false), m_daemonConnected(false), pid(-1)
{}

neroshop::DaemonManager::~DaemonManager() {
    if(m_daemonConnected) {
        disconnect();
    }
    #ifdef NEROSHOP_DEBUG    
    std::cout << "daemon manager deleted\n";
    #endif
}

void neroshop::DaemonManager::startDaemonProcess()
{
    // Check if the daemon is already running
    if (m_daemonRunning)
        return;

    #ifdef Q_OS_WIN
    QString program = "neromon.exe";
    #else
    QString program = "./neromon";
    #endif
    QStringList arguments;  // Optional command-line arguments for the daemon

    if(isDaemonRunningAlready()) {
        std::cout << "\033[35m" << program.toStdString() << " was already running in the background\033[0m\n";
        setDaemonRunning(true);
        return;
    }
    
    // Start the daemon process
    QProcess daemonProcess;
    daemonProcess.start(program, arguments, QIODevice::ReadWrite);
    if (daemonProcess.waitForStarted()) {
        // Daemon process started successfully
        setDaemonRunning(true);
    } else {
        // Failed to start the daemon process
    }
}

void neroshop::DaemonManager::startDaemonProcessDetached() {
    // Check if the daemon is already running
    if (m_daemonRunning)
        return;

    #ifdef Q_OS_WIN
    QString program = "neromon.exe";
    #else
    QString program = "./neromon";
    #endif

    if(isDaemonRunningAlready()) {
        std::cout << "\033[35m" << program.toStdString() << " was already running in the background\033[0m\n";
        setDaemonRunning(true);
        // Connect to daemon server
        std::thread connectionThread([this]() {
            /*// Wait for a certain period before attempting to connect
            std::chrono::seconds delay(10);  // Adjust the duration as needed (e.g., 5 seconds)
            std::this_thread::sleep_until(std::chrono::steady_clock::now() + delay);*/
            while (!isDaemonServerBound()) {
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
            }

            // Connect to the daemon
            connect();    
        });
        // Detach the thread, so it runs independently
        connectionThread.detach();
        return;
    }

    // Note: If the calling process exits, the detached process will continue to run unaffected.
    bool success = QProcess::startDetached(program, {}, QString(), &pid);
    if(!success) { 
        throw std::runtime_error("neroshop daemon process could not be started");
    }
    std::cout << "\033[35mneromon started (pid: " << pid << ")\033[0m\n";
    setDaemonRunning(true);
    
    std::thread connectionThread([this]() {
        // Wait for the port to be bound before attempting to connect
        while (!isDaemonServerBound()) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
        }

        // Connect to the daemon
        connect();    
    });
    // Detach the thread, so it runs independently
    connectionThread.detach();
}

void neroshop::DaemonManager::terminateDaemonProcess() {
    #ifdef Q_OS_WIN
    QString program = "neromon.exe";
    #else
    QString program = "neromon";
    #endif
    
    // Create a QProcess object
    QProcess process;

    // Set the program to the name or path of the daemon process
    process.setProgram(program);

    // Terminate the process
    process.terminate();
    process.waitForFinished();
}

void neroshop::DaemonManager::connect() {    
    // Lock the mutex before accessing the client object
    std::lock_guard<std::mutex> lock(clientMutex);
    
    neroshop::Client * client = neroshop::Client::get_main_client();

    if (!client->connect(NEROSHOP_IPC_DEFAULT_PORT, "127.0.0.1")) {
        onConnectionFailure();
    }

    onConnectionSuccess(); // setDaemonConnected(true) called here
}

void neroshop::DaemonManager::onConnectionSuccess() {
    std::cout << "\033[32mconnected to neromon\033[0m\n";
    setDaemonConnected(true);
}

void neroshop::DaemonManager::onConnectionFailure() {
    std::cout << "Failed to connect to neromon. Retrying...\n";
    QTimer::singleShot(5000, this, [this]() {
        Client* client = Client::get_main_client();
        client->connect(NEROSHOP_IPC_DEFAULT_PORT, "127.0.0.1");
    });
}

void neroshop::DaemonManager::disconnect() {
    Client * client = Client::get_main_client();
    if(client->is_connected()) {
        client->disconnect();
        setDaemonConnected(false);
        setDaemonRunning(false);
        std::cout << "\033[91mdisconnected from neromon\033[0m\n";
    }
}

double neroshop::DaemonManager::getDaemonProgress() const {
    // Return the progress value (between 0 and 1) based on the daemon state
    if(m_daemonConnected) return 1.0;
    if(m_daemonRunning) return 0.5; // Waiting or Launching state
    return 0.0;
}

QString neroshop::DaemonManager::getDaemonStatusText() const {
    // Return the appropriate status text based on the daemon state
    if(m_daemonConnected) return "Connected";
    if(m_daemonRunning) return "Running"; // Waiting or Launching state
    return "Disconnected";
}

void neroshop::DaemonManager::setDaemonRunning(bool running) {
    if (m_daemonRunning != running) {
        m_daemonRunning = running;
        emit daemonRunningChanged(running);
        emit daemonProgressChanged();
        emit daemonStatusTextChanged();
    }
}

void neroshop::DaemonManager::setDaemonConnected(bool connected) {
    if (m_daemonConnected != connected) {
        m_daemonConnected = connected;
        emit daemonConnectedChanged();
        emit daemonProgressChanged();
        emit daemonStatusTextChanged();
    }
}

bool neroshop::DaemonManager::isDaemonRunning() const
{
    return m_daemonRunning;
}

bool neroshop::DaemonManager::isDaemonRunningAlready()
{
    #ifdef Q_OS_WIN
    QString program = "neromon.exe";
    #else
    QString program = "neromon";
    #endif
    
    QProcess process;
    process.start("pgrep", QStringList() << program); // specific to Linux-based systems
    process.waitForFinished();

    return process.exitCode() == 0;
}

bool neroshop::DaemonManager::isDaemonServerBound() {
    QTcpSocket socket;
    socket.abort(); // Ensure the socket is not already in use
    socket.connectToHost("127.0.0.1", NEROSHOP_IPC_DEFAULT_PORT);
    if (socket.waitForConnected(1000)) {
        socket.disconnectFromHost();
        return true;
    }
    return false;    
}

bool neroshop::DaemonManager::isDaemonConnected() const {
    return m_daemonConnected;
}

