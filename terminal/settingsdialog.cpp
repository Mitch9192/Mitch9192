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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QSettings>
#include <QDebug>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

const QString SettingsDialog::SETTINGS_ORG = "GithubRepo-Mitch9192";
const QString SettingsDialog::SETTINGS_APP = "Terminal";
const QString SettingsDialog::SETTINGS_CONNECTION = "connection";
const QString SettingsDialog::SETTINGS_SERIAL_PORT = "serialPort";
const QString SettingsDialog::SETTINGS_BAUD = "baud";
const QString SettingsDialog::SETTINGS_DATA_BITS = "dataBits";
const QString SettingsDialog::SETTINGS_PARITY = "parity";
const QString SettingsDialog::SETTINGS_STOP_BITS = "stopBits";
const QString SettingsDialog::SETTINGS_FLOW_CONTROL = "flowControl";
const QString SettingsDialog::SETTINGS_LOCAL_ECHO = "localEcho";


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog),
    _intValidator(new QIntValidator(0, 4000000, this))
{
    _ui->setupUi(this);

    _ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    _initialConnections();

    _fillPortsParameters();
    _fillPortsInfo();

    _readSettings();
    _applySavedSettings();

    _updateSettings();
}

SettingsDialog::~SettingsDialog() {
    _writeSettings();
    delete _ui;
}

SettingsDialog::Settings
SettingsDialog::settings() const {
    return _currentSettings;
}

void
SettingsDialog::_slot_showPortInfo(int idx) {
    if (idx == -1) { return; }

    const QStringList list = _ui->serialPortInfoListBox->itemData(idx).toStringList();
    _ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    _ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    _ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    _ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    _ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    _ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void
SettingsDialog::_slot_apply() {
    _updateSettings();
    hide();
}

void
SettingsDialog::_slot_checkCustomBaudRatePolicy(int idx) {
    const bool isCustomBaudRate = !_ui->baudRateBox->itemData(idx).isValid();
    _ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        _ui->baudRateBox->clearEditText();
        QLineEdit *edit = _ui->baudRateBox->lineEdit();
        edit->setValidator(_intValidator);
    }
}

void
SettingsDialog::_slot_checkCustomDevicePathPolicy(int idx) {
    const bool isCustomPath = !_ui->serialPortInfoListBox->itemData(idx).isValid();
    _ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath) {
        _ui->serialPortInfoListBox->clearEditText();
    }
}

void
SettingsDialog::_initialConnections() {
    connect(_ui->applyButton, &QPushButton::clicked,
            this, &SettingsDialog::_slot_apply);
    connect(_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::_slot_showPortInfo);
    connect(_ui->baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::_slot_checkCustomBaudRatePolicy);
    connect(_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::_slot_checkCustomDevicePathPolicy);
}

void
SettingsDialog::_fillPortsParameters() {
    _ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    _ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    _ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    _ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    _ui->baudRateBox->addItem(tr("Custom"));

    _ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    _ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    _ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    _ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    _ui->dataBitsBox->setCurrentIndex(3);

    _ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    _ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    _ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    _ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    _ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    _ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    m_ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    _ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    _ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    _ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    _ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void
SettingsDialog::_fillPortsInfo() {
    _ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        _ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    _ui->serialPortInfoListBox->addItem(tr("Custom"));
}


void SettingsDialog::_applySavedSettings() {
    qDebug() << "Apply Settings";

    //Port
    for(auto x = 0; x < _ui->serialPortInfoListBox->count(); x++) {
        if(_savedSettings.name == _ui->serialPortInfoListBox->itemText(x)) {
            _ui->serialPortInfoListBox->setCurrentIndex(x);
            _currentSettings.name = _savedSettings.name;
            qDebug() << "Found Port Settings";
            break;
        }
    }


    //Baud
    if(_savedSettings.baudRate != QSerialPort::BaudRate::UnknownBaud) {
        //-1 for custom.
        for(auto x = 0; x < _ui->baudRateBox->count() - 1; x++) {
            if(_savedSettings.baudRate == _ui->baudRateBox->itemText(x).toInt()) {
                _ui->baudRateBox->setCurrentIndex(x);
                _currentSettings.baudRate = _savedSettings.baudRate;
                _currentSettings.stringBaudRate = QString::number(_savedSettings.baudRate);
                qDebug() << "Found Baud Settings";
                break;
            }
        }
        //TODO: Update GUI to show customer baud.
    }


    //Data Bits
    if(_savedSettings.dataBits != QSerialPort::DataBits::UnknownDataBits) {

        for(auto x = 0; x < _ui->dataBitsBox->count(); x++) {
            if(_savedSettings.dataBits == _ui->dataBitsBox->itemText(x).toInt()) {
                _ui->dataBitsBox->setCurrentIndex(x);
                _currentSettings.dataBits = _savedSettings.dataBits;
                _currentSettings.stringDataBits = _ui->dataBitsBox->currentText();
                qDebug() << "Found Data Bits Settings";
                break;
            }
        }
    }


    //Parity
    if(_savedSettings.parity != QSerialPort::Parity::UnknownParity) {
        for(auto x = 0; x < _ui->dataBitsBox->count(); x++) {
            if(_savedSettings.parity == _ui->dataBitsBox->itemText(x).toInt()) {
                _ui->dataBitsBox->setCurrentIndex(x);
                _currentSettings.parity = _savedSettings.parity;
                _currentSettings.stringParity = _ui->parityBox->currentText();
                qDebug() << "Found Parity Settings";
                break;
            }
        }
    }


    //Stop Bits
    if(_savedSettings.stopBits != QSerialPort::StopBits::UnknownStopBits) {
        for(auto x = 0; x < _ui->stopBitsBox->count(); x++) {
            if(_savedSettings.stopBits == _ui->stopBitsBox->itemText(x).toInt()) {
                _ui->stopBitsBox->setCurrentIndex(x);
                _currentSettings.stopBits = _savedSettings.stopBits;
                _currentSettings.stringStopBits = _ui->stopBitsBox->currentText();
                qDebug() << "Found Stop Bits Settings";
                break;
            }
        }
    }


    //Flow Control
    if(_savedSettings.flowControl != QSerialPort::FlowControl::UnknownFlowControl) {
        for(auto x = 0; x < _ui->flowControlBox->count(); x++) {
            if(_savedSettings.flowControl == _ui->flowControlBox->itemText(x).toInt()) {
                _ui->flowControlBox->setCurrentIndex(x);
                _currentSettings.flowControl = _savedSettings.flowControl;
                _currentSettings.stringFlowControl = _ui->flowControlBox->currentText();
                qDebug() << "Found Flow Control Settings";
                break;
            }
        }
    }

    //Local Echo
    _currentSettings.localEchoEnabled = _savedSettings.localEchoEnabled;


}

void SettingsDialog::_updateSettings()
{
    _currentSettings.name = _ui->serialPortInfoListBox->currentText();

    if (_ui->baudRateBox->currentIndex() == 4) {
        _currentSettings.baudRate = _ui->baudRateBox->currentText().toInt();
    }
    else {
        _currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    _ui->baudRateBox->itemData(_ui->baudRateBox->currentIndex()).toInt());
    }
    _currentSettings.stringBaudRate = QString::number(_currentSettings.baudRate);

    _currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                _ui->dataBitsBox->itemData(_ui->dataBitsBox->currentIndex()).toInt());
    _currentSettings.stringDataBits = _ui->dataBitsBox->currentText();

    _currentSettings.parity = static_cast<QSerialPort::Parity>(
                _ui->parityBox->itemData(_ui->parityBox->currentIndex()).toInt());
    _currentSettings.stringParity = _ui->parityBox->currentText();

    _currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                _ui->stopBitsBox->itemData(_ui->stopBitsBox->currentIndex()).toInt());
    _currentSettings.stringStopBits = _ui->stopBitsBox->currentText();

    _currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                _ui->flowControlBox->itemData(_ui->flowControlBox->currentIndex()).toInt());
    _currentSettings.stringFlowControl = _ui->flowControlBox->currentText();

    _currentSettings.localEchoEnabled = _ui->localEchoCheckBox->isChecked();
}



void
SettingsDialog::_readSettings() {

    QSettings settings(SETTINGS_ORG, SETTINGS_APP);
    qDebug() << "Read Settings";

    settings.beginGroup(SETTINGS_CONNECTION);

    _savedSettings.name = settings.value(SETTINGS_SERIAL_PORT, "").toString();
    qDebug() << "Read: Name: " << _savedSettings.name;
    _savedSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                settings.value(SETTINGS_BAUD, QSerialPort::BaudRate::UnknownBaud).toInt());
    qDebug() << "Read: baudRate: " << _savedSettings.baudRate;
    _savedSettings.dataBits = static_cast<QSerialPort::DataBits>(
                settings.value(SETTINGS_DATA_BITS, QSerialPort::DataBits::UnknownDataBits).toInt());
    qDebug() << "Read: databits: " << _savedSettings.dataBits;
    _savedSettings.parity = static_cast<QSerialPort::Parity>(
                settings.value(SETTINGS_PARITY, QSerialPort::Parity::UnknownParity).toInt());
    qDebug() << "Read: parity: " << _savedSettings.parity;
    _savedSettings.stopBits = static_cast<QSerialPort::StopBits>(
                settings.value(SETTINGS_STOP_BITS, QSerialPort::StopBits::UnknownStopBits).toInt());
    qDebug() << "Read: stopBits: " << _savedSettings.stopBits;
    _savedSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                settings.value(SETTINGS_FLOW_CONTROL, QSerialPort::FlowControl::UnknownFlowControl).toInt());
    qDebug() << "Read: flowControl: " << _savedSettings.flowControl;
    _savedSettings.localEchoEnabled = settings.value(SETTINGS_LOCAL_ECHO, false).toBool();

    settings.endGroup();

}

void
SettingsDialog::_writeSettings() {
    qDebug() << "Write Settings";

    QSettings settings(SETTINGS_ORG, SETTINGS_APP);

    settings.beginGroup(SETTINGS_CONNECTION);

    qDebug() << "Write: Name: " << _currentSettings.name;
    settings.setValue(SETTINGS_SERIAL_PORT, _currentSettings.name);
    qDebug() << "Write: baudRate: " << _currentSettings.baudRate;
    settings.setValue(SETTINGS_BAUD, _currentSettings.baudRate);
    qDebug() << "Write: dataBits: " << _currentSettings.dataBits;
    settings.setValue(SETTINGS_DATA_BITS, _currentSettings.dataBits);
    qDebug() << "Write: parity: " << _currentSettings.parity;
    settings.setValue(SETTINGS_PARITY, _currentSettings.parity);
    qDebug() << "Write: stopBits: " << _currentSettings.stopBits;
    settings.setValue(SETTINGS_STOP_BITS, _currentSettings.stopBits);
    qDebug() << "Write: flowControl: " << _currentSettings.flowControl;
    settings.setValue(SETTINGS_FLOW_CONTROL, _currentSettings.flowControl);
    qDebug() << "Write: localEchoEnabled: " << _currentSettings.localEchoEnabled;
    settings.setValue(SETTINGS_LOCAL_ECHO, _currentSettings.localEchoEnabled);

    settings.endGroup();
}
