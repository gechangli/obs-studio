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
#include "obs.hpp"
#include "obs-app.hpp"
#include <QKeyEvent>
#include <QMessageBox>
#include <QJsonObject>

using namespace std;

XLRegisterDialog::XLRegisterDialog(OBSBasic *parent) :
	QDialog (parent),
	ui(new Ui::XLRegisterDialog),
	m_main(parent),
	m_smsRefreshTimerId(-1),
	m_progressDialog(Q_NULLPTR),
	m_registeredOk(false)
{
	// init ui
	ui->setupUi(this);

	// set title
	setWindowTitle(L("XL.Register.Title"));

	// update button text
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText(L("Register"));
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(L("XL.Register.Already.Has.Account"));

	// listen xgm event
	connect(&m_client, &XgmOA::restOpDone, this, &XLRegisterDialog::onXgmOAResponse);
	connect(&m_client, &XgmOA::restOpFailed, this, &XLRegisterDialog::onXgmOAResponseFailed);
}

void XLRegisterDialog::accept() {
	// validate mobile
	if(!validateMobile()) {
		return;
	}

	// validate password
	if(!validatePassword()) {
		return;
	}

	// validate sms code
	if(!validateSmsCode()) {
		return;
	}

	// register
	m_client.registerUser(ui->mobileEdit->text().trimmed().toStdString(),
						  ui->passwordEdit->text().toStdString(),
						  ui->smsCodeEdit->text().toStdString());

	// show progress dialog
	m_progressDialog = new XLProgressDialog(m_main);
	m_progressDialog->exec();
}

void XLRegisterDialog::reject() {
	// close self
	QDialog::reject();

	// show login dialog
	if(!m_registeredOk) {
		XLLoginDialog login(m_main);
		login.exec();
	}
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

bool XLRegisterDialog::validatePassword() {
	QString pwd = ui->passwordEdit->text();
	QString pwd2 = ui->confirmPasswordEdit->text();
	if(pwd.length() <= 0 && pwd2.length() <= 0) {
		QMessageBox::warning(Q_NULLPTR, L("Warning"), L("XL.Register.Password.Is.Empty"));
		return false;
	} else if(pwd != pwd2) {
		QMessageBox::warning(Q_NULLPTR, L("Warning"), L("XL.Register.Password.Not.Consensus"));
		return false;
	} else {
		return true;
	}
}

bool XLRegisterDialog::validateMobile() {
	QString mobile = ui->mobileEdit->text().trimmed();
	QRegExp regExp("1\\d{10}");
	if(regExp.indexIn(mobile, 0) != -1 && regExp.matchedLength() == mobile.length()) {
		return true;
	} else {
		QMessageBox::warning(Q_NULLPTR, L("Warning"), L("XL.Register.Please.Fill.Mobile"));
		return false;
	}
}

bool XLRegisterDialog::validateSmsCode() {
	QString smsCode = ui->smsCodeEdit->text();
	if(smsCode.length() != 4) {
		QMessageBox::warning(Q_NULLPTR, L("Warning"), L("XL.Register.Sms.Code.Wrong"));
		return false;
	} else {
		return true;
	}
}

void XLRegisterDialog::on_refreshSmsCodeButton_clicked() {
	// validate mobile
	if(!validateMobile()) {
		return;
	}

	// request auth code
	m_client.getAuthCode(ui->mobileEdit->text().trimmed().toStdString());

	// start refresh timer
	m_smsRefreshSeconds = 60;
	m_smsRefreshTimerId = startTimer(1000);
	updateSmsRefreshButtonText();
	ui->refreshSmsCodeButton->setEnabled(false);
}

void XLRegisterDialog::updateSmsRefreshButtonText() {
	if(m_smsRefreshSeconds <= 0) {
		ui->refreshSmsCodeButton->setText(L("XL.Register.Get.Sms.Code"));
	} else {
		ui->refreshSmsCodeButton->setText(QString::asprintf("%s(%ds)", LC("XL.Register.Get.Sms.Code"), m_smsRefreshSeconds));
	}
}

void XLRegisterDialog::timerEvent(QTimerEvent *event) {
	if(event->timerId() == m_smsRefreshTimerId) {
		// if times up, re-enable refresh button
		m_smsRefreshSeconds--;
		if(m_smsRefreshSeconds <= 0) {
			ui->refreshSmsCodeButton->setEnabled(true);
			killTimer(m_smsRefreshTimerId);
			m_smsRefreshTimerId = -1;
		}
		updateSmsRefreshButtonText();
	}
}

void XLRegisterDialog::onXgmOAResponse(XgmOA::XgmRestOp op, QJsonDocument doc) {
	if(op == XgmOA::OP_REGISTER) {
		// perform login if register ok
		m_client.loginByPassword(ui->mobileEdit->text().trimmed().toStdString(),
								 ui->passwordEdit->text().toStdString());
	} else if(op == XgmOA::OP_LOGIN_BY_PASSWORD) {
		// close progress dialog
		m_progressDialog->close();
		delete m_progressDialog;
		m_progressDialog = Q_NULLPTR;

		// save user name and password
		config_t* globalConfig = GetGlobalConfig();
		config_set_string(globalConfig, "XiaomeiLive", "Username", ui->mobileEdit->text().trimmed().toStdString().c_str());
		config_set_string(globalConfig, "XiaomeiLive", "Password", ui->passwordEdit->text().toStdString().c_str());
		config_set_bool(globalConfig, "XiaomeiLive", "RememberPassword", true);
		config_set_bool(globalConfig, "XiaomeiLive", "AutoLogin", true);

		// signal
		m_registeredOk = true;
		emit xgmUserRegistered(ui->mobileEdit->text().trimmed());

		// close self
		close();
	}
}

void XLRegisterDialog::onXgmOAResponseFailed(XgmOA::XgmRestOp op, QNetworkReply::NetworkError errNo, QString errMsg) {
	if(op == XgmOA::OP_GET_AUTO_CODE) {
		// if failed, immediately re-enable refresh button
		ui->refreshSmsCodeButton->setEnabled(true);
		killTimer(m_smsRefreshTimerId);
		m_smsRefreshTimerId = -1;
		m_smsRefreshSeconds = 0;
		updateSmsRefreshButtonText();
	} else if(op == XgmOA::OP_REGISTER) {
		// close progress dialog
		m_progressDialog->close();
		delete m_progressDialog;
		m_progressDialog = Q_NULLPTR;

		// error
		QMessageBox::critical(Q_NULLPTR, L("Error"), QString("%1: %2").arg(L("XL.Register.Failed"), errMsg));
	} else if(op == XgmOA::OP_LOGIN_BY_PASSWORD) {
		// close progress dialog
		m_progressDialog->close();
		delete m_progressDialog;
		m_progressDialog = Q_NULLPTR;

		// log
		blog(LOG_ERROR, "Failed to login: %s", errMsg.toStdString().c_str());

		// show login dialog
		reject();
	}
}