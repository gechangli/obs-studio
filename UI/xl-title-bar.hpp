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

#pragma once

#include <QWidget>
#include <QPixmap>

class QPaintEvent;
class QLabel;
class QPushButton;
class QHBoxLayout;

class XLTitleBar : public QWidget {
	Q_OBJECT

private:
	int m_windowBorderWidth;
	QLabel* m_icon;
	QLabel* m_titleLabel;
	QPushButton* m_minButton;
	QPushButton* m_restoreButton;
	QPushButton* m_maxButton;
	QPushButton* m_closeButton;
	QPoint m_restorePos;
	QSize m_restoreSize;
	bool m_pressed;
	QPoint m_startMovePos;
	QHBoxLayout* m_layout;

private:
	bool hasMaxButton();
	bool hasMinButton();

protected:
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

	// implemented by subclass
	virtual void initCustomUI(QHBoxLayout* layout) = 0;

	// remove icon
	void removeIcon();

signals:
	void windowRequestMinimize();
	void windowRequestRestore();
	void windowRequestMaximize();
	void windowRequestClose();

private slots:
	void onMinClicked();
	void onRestoreClicked();
	void onMaxClicked();
	void onCloseClicked();

public:
	XLTitleBar(QWidget *parent);
	virtual ~XLTitleBar();
	void init();

	// save/load window position and size
	void saveRestoreInfo(const QPoint point, const QSize size);
	void getRestoreInfo(QPoint& point, QSize& size);

	// set window title
	void setWindowTitle(QString title);

	// set icon
	void setIcon(QPixmap icon);

	// implemented by subclass
	virtual int getPreferredHeight() = 0;
	virtual int getButtonWidth() = 0;
	virtual int getButtonHeight() = 0;
};
