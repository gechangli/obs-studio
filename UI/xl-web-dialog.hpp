/******************************************************************************
    Copyright (C) 2014 by Luma <stubma@gmail.com>

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

#include "ui_XLWebDialog.h"
#include <QDialog>
#include <memory>

class XLProgressDialog;

class XLWebDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLWebDialog> ui;
	XLProgressDialog* m_progressDialog;
	bool m_autoSize;

private:
	void showProgressDialog();
	void hideProgressDialog();

protected:
	void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;

public:
	XLWebDialog(QWidget* parent = Q_NULLPTR);
	virtual ~XLWebDialog();

	void openNormal(QUrl initUrl, QString title = "", QSize winSize = QSize(0, 0));
};