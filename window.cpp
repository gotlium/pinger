#include "window.h"

#include <QProcess>
#include <QtGui>
#include <QRegExp>


class PingThread : public QThread {
public:
    Window *window;

protected:
    void run() {
        QStringList list;
        QString text;
        QProcess ping;
        QStringList result;
        QRegExp rx("(\\d+)% loss");

        while(true) {
            if(window->stop) {
                window->stop = false;
                window->StartButton->setEnabled(true);
                window->StopButton->setEnabled(false);
                window->IPInput->setEnabled(true);
                break;
            }

            #ifdef _WIN32
                ping.start("ping.exe " + window->IPInput->text() + " -n 10");
            #else
                ping.start("ping " + window->IPInput->text() + " -c 10");
            #endif

            ping.waitForFinished();
            text = ping.readAll();
            if (!text.isEmpty()) {
                list = text.split("\n");
                if(list.length() < 5) {
                    window->stop = true;
                } else {
                    result = list.filter("unreachable");
                    if(result.length() > 0) {
                        window->stop = true;
                    } else {
                        result = list.filter("loss");
                        if(result.length() > 0) {
                           if(rx.indexIn(result[0], 0) != -1) {
                              if(rx.cap(1) == "100") {
                                 window->stop = true;
                              }
                           }
                        }
                    }
                }
            }
            ping.deleteLater();
        }
    }
};


Window::Window()
{
    QDialog::setWindowFlags(Qt::WindowStaysOnTopHint| Qt::WindowCloseButtonHint | Qt::Tool);

    stop = false;
    createMessageGroupBox();

    createActions();
    createTrayIcon();

    connect(StartButton, SIGNAL(clicked()), this, SLOT(startPing()));
    connect(StopButton, SIGNAL(clicked()), this, SLOT(stopPing()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
               this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(messageGroupBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Pinger v.1.2"));
    resize(215, 100);

    QIcon icon = QIcon(":/images/trash.svg");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->show();
    trayIcon->setToolTip(tr("Pinger v.1.2"));

    setVisible(true);
}

void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            if (this->isVisible()) {
                hide();
            } else {
                show();
                setVisible(true);
            }
            break;
             case QSystemTrayIcon::MiddleClick:
             default:
                 ;
    }
}

void Window::startPing()
{
    // Save ip data
    QFile file("./ip-address.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if(file.isWritable()) {
            const char *ip_address = IPInput->text().toAscii().data();
            file.write(ip_address, qstrlen(ip_address));
        }
    }
    file.close();


    stop = false;
    PingThread *t = new PingThread;
    t->window = this;
    t->start();
    StartButton->setEnabled(false);
    StopButton->setEnabled(true);
    IPInput->setEnabled(false);
}

void Window::stopPing()
{
    StopButton->setEnabled(false);
    stop = true;
}

void Window::createMessageGroupBox()
{
    messageGroupBox = new QGroupBox(tr("Settings"));

    IPLabel = new QLabel(tr("IP-address:"));
    IPInput = new QLineEdit();//tr("192.168.100.250")
    QFile file("./ip-address.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        IPInput->setText(file.readAll());
    }

    StartButton = new QPushButton(tr("Start"));
    StopButton = new QPushButton(tr("Stop"));
    StartButton->setDefault(true);
    StopButton->setEnabled(false);

    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(IPLabel, 1, 0);
    messageLayout->addWidget(IPInput, 1, 1);
    messageLayout->addWidget(StartButton, 2, 0);
    messageLayout->addWidget(StopButton, 2, 1);
    messageGroupBox->setLayout(messageLayout);
}

void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}
