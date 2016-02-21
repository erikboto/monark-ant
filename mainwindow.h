#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onCurrentPowerChanged(quint16 power);
    void onConnectionStatusChanged(bool connected);
    void setCurrentLoad(quint32 load);

private:
    QWidget *m_centralWidget;
    QWidget *m_loadControlWidget;
    QWidget *m_infoWidget;
    QLabel *m_currentLoadLabel;
    QLabel *m_currentPowerLabel;
    QLabel *m_connectionStatus;
    quint32 m_currentLoad;

private slots:
    void onLoadUp100();
    void onLoadUp50();
    void onLoadUp5();
    void onLoadDown100();
    void onLoadDown50();
    void onLoadDown5();

signals:
    void currentLoadChanged(quint32 load);
};

#endif // MAINWINDOW_H
