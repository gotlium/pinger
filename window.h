#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
QT_END_NAMESPACE


class Window : public QDialog
{
    Q_OBJECT

public:
    void setVisible(bool visible);
    Window();
    bool stop;
    QPushButton *StartButton;
    QPushButton *StopButton ;
    QLineEdit *IPInput;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void startPing();
    void stopPing();

private:
    void createMessageGroupBox();
    void createActions();
    void createTrayIcon();

    QGroupBox *messageGroupBox;
    QLabel *IPLabel;

    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
};


#endif
