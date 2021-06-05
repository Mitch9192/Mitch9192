/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QLabel>
#include <QMessageBox>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _status(new QLabel),
    _console(new Console),
    _settings(new SettingsDialog),
    _serial(new QSerialPort(this))
{
    _ui->setupUi(this);
    _console->setEnabled(false);
    setCentralWidget(_console);

    _ui->actionConnect->setEnabled(true);
    _ui->actionDisconnect->setEnabled(false);
    _ui->actionQuit->setEnabled(true);
    _ui->actionConfigure->setEnabled(true);

    _ui->statusBar->addWidget(_status);

    initActionsConnections();

    connect(_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(_console, &Console::getData, this, &MainWindow::writeData);
}

MainWindow::~MainWindow() {
    delete _settings;
    delete _ui;
}

void
MainWindow::openSerialPort() {

    const SettingsDialog::Settings p = _settings->settings();
    _serial->setPortName(p.name);
    _serial->setBaudRate(p.baudRate);
    _serial->setDataBits(p.dataBits);
    _serial->setParity(p.parity);
    _serial->setStopBits(p.stopBits);
    _serial->setFlowControl(p.flowControl);

//    m_serial->setPortName("/dev/tty.usbserial-14213220");
//    m_serial->setBaudRate(QSerialPort::Baud115200);
//    m_serial->setDataBits(QSerialPort::Data8);
//    m_serial->setParity(QSerialPort::NoParity);
//    m_serial->setStopBits(QSerialPort::OneStop);
//    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (_serial->open(QIODevice::ReadWrite)) {
        _console->setEnabled(true);
        _console->setLocalEchoEnabled(p.localEchoEnabled);
        _ui->actionConnect->setEnabled(false);
        _ui->actionDisconnect->setEnabled(true);
        _ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    }
    else {
        QMessageBox::critical(this, tr("Error"), _serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}

void
MainWindow::closeSerialPort() {
    if(_serial->isOpen()) {
        _serial->close();
    }

    _console->setEnabled(false);
    _ui->actionConnect->setEnabled(true);
    _ui->actionDisconnect->setEnabled(false);
    _ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

void
MainWindow::about() {
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

void
MainWindow::writeData(const QByteArray &data) {
    _serial->write(data);
}

void
MainWindow::readData() {
    const QByteArray data = _serial->readAll();
    _console->putData(data);
}

void
MainWindow::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), _serial->errorString());
        closeSerialPort();
    }
}

void
MainWindow::initActionsConnections() {
    connect(_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(_ui->actionConfigure, &QAction::triggered, _settings, &SettingsDialog::show);
    connect(_ui->actionClear, &QAction::triggered, _console, &Console::clear);
    connect(_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void
MainWindow::showStatusMessage(const QString &message) {
    _status->setText(message);
}
