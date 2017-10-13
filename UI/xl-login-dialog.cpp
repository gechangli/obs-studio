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

#include "obs-app.hpp"
#include "xl-login-dialog.hpp"
#include "xl-register-dialog.hpp"
#include "xl-progress-dialog.hpp"
#include "window-basic-main.hpp"
#include <QMessageBox>
#include <QKeyEvent>

XLLoginDialog::XLLoginDialog(OBSBasic *parent) :
	QDialog (parent),
	ui(new Ui::XLLoginDialog),
	m_main(parent),
	m_smsRefreshTimerId(-1),
	m_progressDialog(Q_NULLPTR),
	m_loggedIn(false)
{
	// init ui
	ui->setupUi(this);

	// set title
	setWindowTitle(L("SignIn"));

	// listen xgm event
	connect(&m_client, &XgmOA::restOpDone, this, &XLLoginDialog::onXgmOAResponse);
	connect(&m_client, &XgmOA::restOpFailed, this, &XLLoginDialog::onXgmOAResponseFailed);

	// update button text
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText(L("SignIn"));
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(L("SignUp"));

	// pre-fill ui
	config_t* globalConfig = GetGlobalConfig();
	QString username = config_get_string(globalConfig, "XiaomeiLive", "Username");
	QString password = config_get_string(globalConfig, "XiaomeiLive", "Password");
	bool rememberPassword = config_get_bool(globalConfig, "XiaomeiLive", "RememberPassword");
	bool autoLogin = config_get_bool(globalConfig, "XiaomeiLive", "AutoLogin");
	ui->mobileEdit->setText(username);
	ui->passwordEdit->setText(password);
	ui->rememberPasswordCheckBox->setChecked(rememberPassword);
	ui->autoLoginCheckBox->setChecked(autoLogin);

	// by default, use password
	ui->tabWidget->setCurrentIndex(0);
}

void XLLoginDialog::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			break;
		default:
			QDialog::keyPressEvent(event);
			break;
	}
}

void XLLoginDialog::timerEvent(QTimerEvent *event) {
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

void XLLoginDialog::accept() {
	// validate mobile
	if(!validateMobile()) {
		return;
	}

	// validate sms code
	if(!validateSmsCode()) {
		return;
	}

	// login
	m_client.loginByAuthCode(ui->mobileEdit->text().trimmed().toStdString(),
							 ui->smsCodeEdit->text().toStdString());

	// progress
	showProgressDialog();
}

void XLLoginDialog::reject() {
	// close self
	QDialog::reject();

	// show login dialog
	if(!m_loggedIn) {
		XLRegisterDialog reg(m_main);
		connect(&reg, &XLRegisterDialog::xgmUserLoggedIn, m_main, &OBSBasic::xgmUserLoggedIn);
		reg.exec();
	}
}

void XLLoginDialog::showProgressDialog() {
	m_progressDialog = new XLProgressDialog(m_main);
	m_progressDialog->exec();
}

void XLLoginDialog::hideProgressDialog() {
	m_progressDialog->close();
	delete m_progressDialog;
	m_progressDialog = Q_NULLPTR;
}

bool XLLoginDialog::validateMobile() {
	QString mobile = ui->mobileEdit->text().trimmed();
	QRegExp regExp("1\\d{10}");
	if(regExp.indexIn(mobile, 0) != -1 && regExp.matchedLength() == mobile.length()) {
		return true;
	} else {
		QMessageBox::warning(Q_NULLPTR, L("Warning"), L("XL.Login.Please.Fill.Mobile"));
		return false;
	}
}

bool XLLoginDialog::validateSmsCode() {
	QString smsCode = ui->smsCodeEdit->text();
	if(smsCode.length() != 4) {
		QMessageBox::warning(Q_NULLPTR, L("Warning"), L("XL.Login.Sms.Code.Wrong"));
		return false;
	} else {
		return true;
	}
}

void XLLoginDialog::on_refreshSmsCodeButton_clicked() {
	// validate mobile
	if(!validateMobile()) {
		return;
	}

	// request auth code
	m_client.getAuthCode(ui->mobileEdit->text().trimmed().toStdString(), XgmOA::AT_LOGIN);

	// show progress
	showProgressDialog();
}

void XLLoginDialog::updateSmsRefreshButtonText() {
	if(m_smsRefreshSeconds <= 0) {
		ui->refreshSmsCodeButton->setText(L("XL.Login.Get.Sms.Code"));
	} else {
		ui->refreshSmsCodeButton->setText(QString::asprintf("%s(%ds)", LC("XL.Login.Get.Sms.Code"), m_smsRefreshSeconds));
	}
}

void XLLoginDialog::onXgmOAResponse(XgmOA::XgmRestOp op, QJsonDocument doc) {
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
		ui->refreshSmsCodeButton->setEnabled(false);
	} else if(op == XgmOA::OP_LOGIN_BY_AUTHCODE) {
		// close progress dialog
		hideProgressDialog();

		// save user name and password
		config_t* globalConfig = GetGlobalConfig();
		config_set_string(globalConfig, "XiaomeiLive", "Username", ui->mobileEdit->text().trimmed().toStdString().c_str());
		config_set_bool(globalConfig, "XiaomeiLive", "AutoLogin", ui->autoLoginCheckBox->isChecked());
		config_save_safe(globalConfig, "tmp", Q_NULLPTR);

		// signal
		m_loggedIn = true;
		emit xgmUserLoggedIn(ui->mobileEdit->text().trimmed());

		// close
		close();
	}
}

void XLLoginDialog::onXgmOAResponseFailed(XgmOA::XgmRestOp op, QNetworkReply::NetworkError errNo, QString errMsg) {
	if(QObject::sender() != &m_client) {
		return;
	}

	if(op == XgmOA::OP_GET_AUTO_CODE) {
		// hide progress dialog
		hideProgressDialog();

		// if failed, immediately re-enable refresh button
		ui->refreshSmsCodeButton->setEnabled(true);
		killTimer(m_smsRefreshTimerId);
		m_smsRefreshTimerId = -1;
		m_smsRefreshSeconds = 0;
		updateSmsRefreshButtonText();
	} else if(op == XgmOA::OP_LOGIN_BY_AUTHCODE) {
		// close progress dialog
		hideProgressDialog();

		// warning
		QMessageBox::critical(Q_NULLPTR, L("Error"), errMsg);
	}
}