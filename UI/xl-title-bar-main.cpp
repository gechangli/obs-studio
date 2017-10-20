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

#include <QLabel>
#include <QSpacerItem>
#include "xl-title-bar-main.hpp"
#include "window-basic-main.hpp"
#include "xl-clickable-label.hpp"
#include "xl-user-popup-widget.hpp"

#define TITLE_HEIGHT 48
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 30

XLTitleBarMain::XLTitleBarMain(QWidget *parent) :
	XLTitleBar(parent) {
}

XLTitleBarMain::~XLTitleBarMain() {
}

int XLTitleBarMain::getPreferredHeight() {
	return TITLE_HEIGHT;
}

int XLTitleBarMain::getButtonWidth() {
	return BUTTON_WIDTH;
}

int XLTitleBarMain::getButtonHeight() {
	return BUTTON_HEIGHT;
}

void XLTitleBarMain::initCustomUI(QHBoxLayout* layout) {
	// set icon
	setIcon(QPixmap(":/res/images/logo.png"));

	// create label for user
	m_userLabel = new XLClickableLabel;
	m_arrowLabel = new XLClickableLabel;

	// set arrow icon
	m_arrowLabel->setPixmap(QPixmap(":/res/images/drop_down_arrow.png"));

	// set text, font and color of user label
	m_userLabel->setText(L("NotLogged"));

	// event
	connect(m_userLabel, &XLClickableLabel::clicked, this, &XLTitleBarMain::onUserLabelClicked);
	connect(m_arrowLabel, &XLClickableLabel::clicked, this, &XLTitleBarMain::onUserLabelClicked);

	// add
	layout->addWidget(m_userLabel);
	layout->addWidget(m_arrowLabel);
	layout->addItem(new QSpacerItem(24, 1, QSizePolicy::Fixed, QSizePolicy::Fixed));
}

void XLTitleBarMain::onUserLabelClicked() {
	XLUserPopupWidget* popup = XLUserPopupWidget::instance();
	popup->setParent(this);
	popup->setReferenceWidget(m_arrowLabel);
	if (popup->isHidden()) {
		popup->setWindowFlags(Qt::Popup);
		popup->show();
	} else {
		popup->hide();
	}
}

void XLTitleBarMain::setUsername(QString username) {
	m_userLabel->setText(L("XL.Title.Current.User") + username);
}

void XLTitleBarMain::removeUsername() {
	m_userLabel->setText(L("NotLogged"));
}