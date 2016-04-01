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

#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_currentLoad(0)
{

    QString styleSheetRed("font-size: 18pt; font-weight: bold; color: #ff0000;");
    QString styleSheetGreen("font-size: 18pt; font-weight: bold; color: #00ff00;");
    QString styleSheetBlue("font-size: 18pt; font-weight: bold; color: #0000ff;");
    QString styleSheetBlack("font-size: 18pt; font-weight: bold; color: #000000;");

    m_centralWidget = new QWidget();

    m_loadControlWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QHBoxLayout *loadControlLayout = new QHBoxLayout();

    QPushButton *loadUp100 = new QPushButton("+100W");
    QPushButton *loadUp50 = new QPushButton("+50W");
    QPushButton *loadUp5 = new QPushButton("+5W");
    QPushButton *loadDown100 = new QPushButton("-100W");
    QPushButton *loadDown50 = new QPushButton("-50W");
    QPushButton *loadDown5 = new QPushButton("-5W");

    loadUp100->setStyleSheet(styleSheetRed);
    loadUp50->setStyleSheet(styleSheetRed);
    loadUp5->setStyleSheet(styleSheetRed);
    loadDown100->setStyleSheet(styleSheetBlue);
    loadDown50->setStyleSheet(styleSheetBlue);
    loadDown5->setStyleSheet(styleSheetBlue);

    loadUp100->setFixedHeight(100);
    loadUp50->setFixedHeight(100);
    loadUp5->setFixedHeight(100);
    loadDown100->setFixedHeight(100);
    loadDown50->setFixedHeight(100);
    loadDown5->setFixedHeight(100);

    connect(loadDown5, SIGNAL(clicked(bool)), this, SLOT(onLoadDown5()));
    connect(loadDown50, SIGNAL(clicked(bool)), this, SLOT(onLoadDown50()));
    connect(loadDown100, SIGNAL(clicked(bool)), this, SLOT(onLoadDown100()));
    connect(loadUp5, SIGNAL(clicked(bool)), this, SLOT(onLoadUp5()));
    connect(loadUp50, SIGNAL(clicked(bool)), this, SLOT(onLoadUp50()));
    connect(loadUp100, SIGNAL(clicked(bool)), this, SLOT(onLoadUp100()));

    loadControlLayout->addWidget(loadUp100);
    loadControlLayout->addWidget(loadUp50);
    loadControlLayout->addWidget(loadUp5);
    loadControlLayout->addWidget(loadDown5);
    loadControlLayout->addWidget(loadDown50);
    loadControlLayout->addWidget(loadDown100);

    m_loadControlWidget->setLayout(loadControlLayout);


    m_infoWidget = new QWidget();
    QHBoxLayout *infoLayout = new QHBoxLayout();
    m_currentLoadLabel = new QLabel();
    m_currentLoadLabel->setText("Target Power: 0");
    m_currentLoadLabel->setStyleSheet(styleSheetBlack);

    m_currentPowerLabel = new QLabel();
    m_currentPowerLabel->setText("Current Power: 0");
    m_currentPowerLabel->setStyleSheet(styleSheetBlack);

    m_connectionStatus = new QLabel();
    m_connectionStatus->setText("Disconnected");
    m_connectionStatus->setStyleSheet(styleSheetRed);

    infoLayout->addWidget(m_currentLoadLabel);
    infoLayout->addWidget(m_currentPowerLabel);
    infoLayout->addWidget(m_connectionStatus);
    m_infoWidget->setLayout(infoLayout);

    layout->addWidget(m_loadControlWidget);
    layout->addWidget(m_infoWidget);

    m_centralWidget->setLayout(layout);

    this->setCentralWidget(m_centralWidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::onCurrentPowerChanged(quint16 power)
{
    if (m_currentPowerLabel)
    {
        m_currentPowerLabel->setText(QString("Current Power: %1").arg(power));
    }
}

void MainWindow::onConnectionStatusChanged(bool connected)
{

    static QString styleSheetRed("font-size: 18pt; font-weight: bold; color: #ff0000;");
    static QString styleSheetGreen("font-size: 18pt; font-weight: bold; color: #00ff00;");
    m_connectionStatus->setText(connected ? "Connected" : "Disconnected");
    m_connectionStatus->setStyleSheet(connected ? styleSheetGreen : styleSheetRed);
}

void MainWindow::setCurrentLoad(quint32 load)
{
    m_currentLoad = load;
    m_currentLoadLabel->setText(QString("Target Power: %1").arg(load));
    emit currentLoadChanged(m_currentLoad);
}

void MainWindow::onLoadDown5()
{
    setCurrentLoad(qint32(m_currentLoad-5)>0 ? m_currentLoad-5 : 0);
}

void MainWindow::onLoadDown50()
{
    setCurrentLoad(qint32(m_currentLoad-50)>0 ? m_currentLoad-50 : 0);
}

void MainWindow::onLoadDown100()
{
    setCurrentLoad(qint32(m_currentLoad-100)>0 ? m_currentLoad-100 : 0);
}

void MainWindow::onLoadUp100()
{
    setCurrentLoad(m_currentLoad+100);
}

void MainWindow::onLoadUp50()
{
    setCurrentLoad(m_currentLoad+50);
}

void MainWindow::onLoadUp5()
{
    setCurrentLoad(m_currentLoad+5);
}

