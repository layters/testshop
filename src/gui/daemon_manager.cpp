#include "daemon_manager.hpp"

#include <QFile>

#include <iostream>
#include <chrono>
#include <thread>

#include "../neroshop_config.hpp"
#include "../core/protocol/transport/client.hpp"

neroshop::DaemonManager::DaemonManager(QObject *parent)
    : QObject{parent}, m_daemonRunning(false), m_daemonConnected(false)//, pid(-1)
{}

neroshop::DaemonManager::~DaemonManager() {
    if(m_daemonConnected) {
        disconnect();
    }
    #ifdef NEROSHOP_DEBUG    
    std::cout << "daemon manager deleted\n";
    #endif
}

qint64 neroshop::DaemonManager::pid(-1);

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
    QString program;
    if (QFile::exists("neromon.AppImage")) {
        program = "./neromon.AppImage";
    } else {
        program = "./neromon";
    }
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
    std::cout << "\033[35;1mneromon started (pid: " << pid << ")\033[0m\n";
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
        return;
    }

    onConnectionSuccess(); // setDaemonConnected(true) called here
}

void neroshop::DaemonManager::onConnectionSuccess() {
    std::cout << "\033[32;1mconnected to neromon\033[0m\n";
    setDaemonConnected(true);
}

void neroshop::DaemonManager::onConnectionFailure() {
    std::cout << "Failed to connect to neromon. Retrying...\n";

    std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::seconds(5));
    Client* client = Client::get_main_client();
    client->connect(NEROSHOP_IPC_DEFAULT_PORT, "127.0.0.1");
}

void neroshop::DaemonManager::disconnect() {
    Client * client = Client::get_main_client();
    if(client->is_connected()) {
        client->disconnect();
        setDaemonConnected(false);
        setDaemonRunning(false);
        std::cout << "\033[91;1mdisconnected from neromon\033[0m\n";
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
    if(m_daemonRunning) return "Launching"; // Waiting or Launching state
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
    QString program = "neromon"; // will work with AppImage as well
    #endif
    
    QProcess process;
    process.start("pgrep", QStringList() << program); // specific to Linux-based systems
    process.waitForFinished();

    return process.exitCode() == 0;
}

bool neroshop::DaemonManager::isDaemonServerBound() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(NEROSHOP_IPC_DEFAULT_PORT);

    int result = bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    close(sockfd);

    return (result == -1); // If bind fails, daemon is already bound to port
}

bool neroshop::DaemonManager::isDaemonConnected() const {
    return m_daemonConnected;
}

