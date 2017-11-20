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

#include "xl-register-dialog.hpp"
#include "window-basic-main.hpp"
#include "xl-login-dialog.hpp"
#include "xl-progress-dialog.hpp"
#include "xl-web-dialog.hpp"
#include <QKeyEvent>
#include "xl-util.hpp"

using namespace std;

XLRegisterDialog::XLRegisterDialog(OBSBasic *parent) :
	QDialog (parent, Qt::FramelessWindowHint),
	ui(new Ui::XLRegisterDialog),
	m_main(parent),
	m_smsRefreshTimerId(-1),
	m_progressDialog(Q_NULLPTR),
	m_registeredOk(false) {
	// init ui
	ui->setupUi(this);
	clearErrorMessage();

	// disable focus rect
	ui->mobileEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
	ui->smsCodeEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

	// listen xgm event
	connect(&m_client, &XgmOA::restOpDone, this, &XLRegisterDialog::onXgmOAResponse);
	connect(&m_client, &XgmOA::restOpFailed, this, &XLRegisterDialog::onXgmOAResponseFailed);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-register-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

void XLRegisterDialog::accept() {
	// clear error message
	clearErrorMessage();

	// validate mobile
	if(!validateMobile()) {
		return;
	}

	// validate sms code
	if(!validateSmsCode()) {
		return;
	}

	// validate agreement
	if(!validateAgreement()) {
		return;
	}

	// register
	m_client.registerUser(ui->mobileEdit->text().trimmed().toStdString(),
						  ui->smsCodeEdit->text().toStdString());

	// show progress dialog
	showProgressDialog();
}

void XLRegisterDialog::reject() {
	// close self
	QDialog::reject();

	// show login dialog
	if(!m_registeredOk) {
		XLLoginDialog login(m_main);
		connect(&login, &XLLoginDialog::xgmUserLoggedIn, m_main, &OBSBasic::xgmUserLoggedIn);
		login.exec();
	}
}

void XLRegisterDialog::showProgressDialog() {
	m_progressDialog = new XLProgressDialog(m_main);
	m_progressDialog->exec();
}

void XLRegisterDialog::hideProgressDialog() {
	m_progressDialog->close();
	delete m_progressDialog;
	m_progressDialog = Q_NULLPTR;
}

void XLRegisterDialog::showErrorMessage(QString msg) {
	ui->warningIconLabel->setVisible(true);
	ui->warningLabel->setVisible(true);
	ui->warningLabel->setText(msg);
}

void XLRegisterDialog::clearErrorMessage() {
	ui->warningIconLabel->setVisible(false);
	ui->warningLabel->setVisible(false);
}

void XLRegisterDialog::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			break;
		default:
			QDialog::keyPressEvent(event);
			break;
	}
}

bool XLRegisterDialog::validateAgreement() {
	if(ui->licenseCheckbox->isChecked()) {
		return true;
	} else {
		showErrorMessage(L("XL.Error.Please.Accept.License"));
		return false;
	}
}

bool XLRegisterDialog::validateMobile() {
	QString mobile = ui->mobileEdit->text().trimmed();
	QRegExp regExp("1\\d{10}");
	if(regExp.indexIn(mobile, 0) != -1 && regExp.matchedLength() == mobile.length()) {
		return true;
	} else {
		showErrorMessage(L("XL.Error.Please.Fill.Mobile"));
		return false;
	}
}

bool XLRegisterDialog::validateSmsCode() {
	QString smsCode = ui->smsCodeEdit->text();
	if(smsCode.length() != 4) {
		showErrorMessage(L("XL.Error.Sms.Code.Wrong"));
		return false;
	} else {
		return true;
	}
}

void XLRegisterDialog::on_getSmsButton_clicked() {
	// validate mobile
	if(!validateMobile()) {
		return;
	}

	// request auth code
	m_client.getAuthCode(ui->mobileEdit->text().trimmed().toStdString(), XgmOA::AT_REGISTER);

	// show progress
	showProgressDialog();
}

void XLRegisterDialog::on_signUpButton_clicked() {
	accept();
}

void XLRegisterDialog::on_signInButton_clicked() {
	reject();
}

void XLRegisterDialog::on_licenseLabel_clicked() {
	QUrl url("http://101.201.253.175:8000/admin/live/login/protocol");
	XLWebDialog* web = new XLWebDialog();
	web->openUrl(url, L("User.Agreement"));
}

void XLRegisterDialog::updateSmsRefreshButtonText() {
	if(m_smsRefreshSeconds <= 0) {
		ui->getSmsButton->setText(L("Get.Sms.Code"));
	} else {
		ui->getSmsButton->setText(QString::asprintf("%s(%ds)", LC("Get.Sms.Code"), m_smsRefreshSeconds));
	}
}

void XLRegisterDialog::timerEvent(QTimerEvent *event) {
	if(event->timerId() == m_smsRefreshTimerId) {
		// if times up, re-enable refresh button
		m_smsRefreshSeconds--;
		if(m_smsRefreshSeconds <= 0) {
			ui->getSmsButton->setEnabled(true);
			killTimer(m_smsRefreshTimerId);
			m_smsRefreshTimerId = -1;
		}
		updateSmsRefreshButtonText();
	}
}

void XLRegisterDialog::onXgmOAResponse(XgmOA::XgmRestOp op, QJsonDocument doc) {
	if(QObject::sender() != &m_client) {
		return;
	}

	if(op == XgmOA::OP_GET_AUTO_CODE) {
		// close progress dialog
		hideProgressDialog();

		// start refresh timer
		m_smsRefreshSeconds = 60;
		m_smsRefreshTimerId = startTimer(1000);
		updateSmsRefreshButtonText();
		ui->getSmsButton->setEnabled(false);
	} else if(op == XgmOA::OP_REGISTER) {
		// signal
		m_registeredOk = true;
		emit xgmUserRegistered(ui->mobileEdit->text().trimmed());

		// show login dialog
		XLLoginDialog login(m_main);
		connect(&login, &XLLoginDialog::xgmUserLoggedIn, m_main, &OBSBasic::xgmUserLoggedIn);
		login.exec();

		// close
		close();
	}
}

void XLRegisterDialog::onXgmOAResponseFailed(XgmOA::XgmRestOp op, QNetworkReply::NetworkError errNo, QString errMsg) {
	if(QObject::sender() != &m_client) {
		return;
	}

	if(op == XgmOA::OP_GET_AUTO_CODE) {
		// hide progress dialog
		hideProgressDialog();

		// error
		showErrorMessage(errMsg);

		// if failed, immediately re-enable refresh button
		ui->getSmsButton->setEnabled(true);
		killTimer(m_smsRefreshTimerId);
		m_smsRefreshTimerId = -1;
		m_smsRefreshSeconds = 0;
		updateSmsRefreshButtonText();
	} else if(op == XgmOA::OP_REGISTER) {
		// close progress dialog
		hideProgressDialog();

		// error
		showErrorMessage(QString("%1: %2").arg(L("XL.Error.Sign.Up.Failed"), errMsg));
	}
}