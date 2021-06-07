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

#include "console.h"

#include <QScrollBar>
#include <QDebug>

Console::Console(QWidget *parent) : QPlainTextEdit(parent) {
    document()->setMaximumBlockCount(5000);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
}

void
Console::putData(const QByteArray &data) {

    //the QPlainTextEdit doesn't like delete char sent to us.
    //We intercept and manually delete the char.
    for(auto c : data) {
        if(c == char(8)) {
            this->textCursor().deletePreviousChar();
        }
        else {
            insertPlainText(QString(c));
        }
    }

    qDebug() << "Data: " << data;

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void
Console::setLocalEchoEnabled(bool set) {
    m_localEchoEnabled = set;
}

void Console::keyPressEvent(QKeyEvent *e)
{
    QByteArray a;

    switch (e->key()) {

    case Qt::Key_Up:
        a.push_back(0x1E);
        break;

    case Qt::Key_Down:
        a.push_back(0x1F);
        break;

    case Qt::Key_Left:
        a.push_back(0x1C);
        break;

    case Qt::Key_Right:
        a.push_back(0x1D);
        break;

    case Qt::Key_Enter:
        a.push_back(0xD); //use carriage return
        break;

    default:
        a.push_back(e->text().toLocal8Bit());
        break;

    }

    qDebug() << "Ascii: " << a << " key Value: " << e->key();
    emit getData(a);
}

//void Console::keyPressEvent(QKeyEvent *e)
//{


//    switch (e->key()) {
//    case Qt::Key_Backspace:
//        QPlainTextEdit::keyPressEvent(e);
//        _buffer.chop(1);
//        break;

//    case Qt::Key_Left:
//        QPlainTextEdit::keyPressEvent(e);
//        if(_bufferIndex >= 0) {
//            _bufferIndex--;
//        }

//        break;

//    case Qt::Key_Right:
//        QPlainTextEdit::keyPressEvent(e);
//        if(_bufferIndex < _buffer.size()-1 ) {
//            _bufferIndex++;
//        }
//        break;

//    case Qt::Key_Up:
//        qDebug() << "commandHistoryIndex: " << _commandHistoryIndex;

//        if(_commandHistoryIndex >= 0) {
//            backspace(_buffer.size());
//            this->putData(_commandHistory.at(_commandHistoryIndex));
//            _buffer.clear();
//            _buffer = _commandHistory.at(_commandHistoryIndex);
//            if(--_commandHistoryIndex < 0) { _commandHistoryIndex = 0; }

//        }

//        break;


//    case Qt::Key_Down:
//        qDebug() << "commandHistoryIndex: " << _commandHistoryIndex;

//        if(_commandHistoryIndex < (int)_commandHistory.size()) {
//            backspace(_buffer.size());
//            this->putData(_commandHistory.at(_commandHistoryIndex));
//            _buffer.clear();
//            _buffer = _commandHistory.at(_commandHistoryIndex);

//            if(++_commandHistoryIndex >= (int)_commandHistory.size()) {
//                _commandHistoryIndex = _commandHistory.size() - 1;
//            }
//        }

//        break;


//    case Qt::Key_Return:
//        _commandHistory.push_back(_buffer); //add before the \n
//        _commandHistoryIndex = _commandHistory.size() - 1;

//        if(_bufferIndex != _buffer.size()) {
//            qDebug() << "Move Cursor";
//            this->moveCursor(QTextCursor::End);
//        }

//        QPlainTextEdit::keyPressEvent(e);
//        _buffer.push_back(e->text().toLocal8Bit());
//        emit getData(_buffer);

//        _buffer.clear();
//        _bufferIndex = 0;
//        break;

//    default:
//        QPlainTextEdit::keyPressEvent(e);

//        if(_bufferIndex == _buffer.size()) {
//            qDebug() << "add";
//            _buffer.push_back(e->text().toLocal8Bit());
//        }
//        else {
//            qDebug() << "insert";
//            _buffer.insert(_bufferIndex, e->text().toLocal8Bit());
//        }

//        _bufferIndex++;

//    }

//    qDebug() << "Size: " << _buffer.size();
//}

void
Console::backspace(size_t count) {
    for(size_t x = 0; x < count; x++) {
        this->textCursor().deletePreviousChar();
    }
}

