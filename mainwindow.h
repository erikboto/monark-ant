/*
 * Copyright (c) 2016 Erik Botö (erik.boto@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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
