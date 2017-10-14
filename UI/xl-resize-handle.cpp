/******************************************************************************
    Copyright (C) 2013 by luma <stubma@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "xl-resize-handle.hpp"
#include <QMouseEvent>
#include "window-basic-main.hpp"

XLResizeHandle::XLResizeHandle(QWidget* parent, Qt::WindowFlags f) :
	QLabel(parent, f) {
}

XLResizeHandle::~XLResizeHandle() {
}

void XLResizeHandle::mousePressEvent(QMouseEvent* event) {
	m_startPos = event->globalPos();
	m_startSize = App()->GetMainWindow()->size();
}

void XLResizeHandle::mouseMoveEvent(QMouseEvent *event) {
	QPoint delta = event->globalPos() - m_startPos;
	QSize newSize = m_startSize + QSize(delta.x(), delta.y());
	App()->GetMainWindow()->resize(newSize);
}

void XLResizeHandle::mouseReleaseEvent(QMouseEvent *event) {

}