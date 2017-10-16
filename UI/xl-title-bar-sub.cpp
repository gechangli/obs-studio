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

#include "xl-title-bar-sub.hpp"
#include <QHBoxLayout>

#define TITLE_HEIGHT 32
#define BUTTON_HEIGHT 26
#define BUTTON_WIDTH 26

XLTitleBarSub::XLTitleBarSub(QWidget *parent) :
	XLTitleBar(parent) {
}

XLTitleBarSub::~XLTitleBarSub() {
}

int XLTitleBarSub::getPreferredHeight() {
	return TITLE_HEIGHT;
}

int XLTitleBarSub::getButtonWidth() {
	return BUTTON_WIDTH;
}

int XLTitleBarSub::getButtonHeight() {
	return BUTTON_HEIGHT;
}

void XLTitleBarSub::initCustomUI(QHBoxLayout* layout) {
	removeIcon();
	layout->setContentsMargins(16, 0, 5, 0);
}