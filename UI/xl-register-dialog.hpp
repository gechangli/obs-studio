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

#include "ui_XLRegisterDialog.h"
#include "xiguamei-oa.hpp"
#include <QDialog>

class OBSBasic;
class XLProgressDialog;

class XLRegisterDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLRegisterDialog> ui;
	OBSBasic* m_main;
	XgmOA m_client;
	int m_smsRefreshTimerId;
	int m_smsRefreshSeconds;
	XLProgressDialog* m_progressDialog;
	bool m_registeredOk;

private:
	void updateSmsRefreshButtonText();
	bool validateMobile();
	bool validatePassword();
	bool validateSmsCode();

private slots:
	void on_refreshSmsCodeButton_clicked();
	void onXgmOAResponse(XgmOA::XgmRestOp op, QJsonDocument doc);
	void onXgmOAResponseFailed(XgmOA::XgmRestOp op, QNetworkReply::NetworkError errNo, QString errMsg);

signals:
	void xgmUserRegistered(QString username);

protected:
	void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
	void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

public:
	XLRegisterDialog(OBSBasic *parent);

	// slot override
	void accept() Q_DECL_OVERRIDE;
	void reject() Q_DECL_OVERRIDE;
};