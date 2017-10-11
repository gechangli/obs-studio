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

#include <QPainter>
#include <QPainter>
#include <QPixmap>
#include "obs-app.hpp"
#include "window-basic-main.hpp"
#include "window-basic-main-outputs.hpp"
#include "xl-status-bar.hpp"

XLStatusBar::XLStatusBar(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLStatusBar),
	transparentPixmap(20, 20),
	greenPixmap(20, 20),
	grayPixmap(20, 20),
	redPixmap(20, 20) {
	// init ui
	ui->setupUi(this);

	ui->liveTimeLabel->setText(QString("LIVE: 00:00:00"));
	ui->recordTimeLabel->setText(QString("REC: 00:00:00"));
	ui->cpuLabel->setText(QString("CPU: 0.0%, 0.00 fps"));
	ui->delayInfoLabel->setText("");
	ui->dropFrameLabel->setText("");
	ui->msgLabel->setText("");
	ui->networkSpeedLabel->setText("");

	transparentPixmap.fill(QColor(0, 0, 0, 0));
	greenPixmap.fill(QColor(0, 255, 0));
	grayPixmap.fill(QColor(72, 72, 72));
	redPixmap.fill(QColor(255, 0, 0));

	ui->networkSpeedIconLabel->setPixmap(transparentPixmap);
}

XLStatusBar::~XLStatusBar() {
}

void XLStatusBar::paintEvent(QPaintEvent* event) {
	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLStatusBar::Activate()
{
	if (!active) {
		refreshTimer = new QTimer(this);
		connect(refreshTimer, SIGNAL(timeout()),
				this, SLOT(UpdateStatusBar()));

		int skipped = video_output_get_skipped_frames(obs_get_video());
		int total   = video_output_get_total_frames(obs_get_video());

		totalStreamSeconds = 0;
		totalRecordSeconds = 0;
		lastSkippedFrameCount = 0;
		startSkippedFrameCount = skipped;
		startTotalFrameCount = total;

		refreshTimer->start(1000);
		active = true;

		if (streamOutput) {
			ui->networkSpeedIconLabel->setPixmap(grayPixmap);
		}
	}
}

void XLStatusBar::Deactivate() {
	OBSBasic *main = dynamic_cast<OBSBasic*>(parent());
	if (!main)
		return;

	if (!streamOutput) {
		ui->liveTimeLabel->setText(QString("LIVE: 00:00:00"));
		totalStreamSeconds = 0;
	}

	if (!recordOutput) {
		ui->recordTimeLabel->setText(QString("REC: 00:00:00"));
		totalRecordSeconds = 0;
	}

	if (!main->Active()) {
		delete refreshTimer;

		ui->delayInfoLabel->setText("");
		ui->dropFrameLabel->setText("");
		ui->networkSpeedLabel->setText("");

		delaySecTotal = 0;
		delaySecStarting = 0;
		delaySecStopping = 0;
		reconnectTimeout = 0;
		active = false;
		overloadedNotify = true;

		ui->networkSpeedIconLabel->setPixmap(transparentPixmap);
	}
}

void XLStatusBar::UpdateDelayMsg()
{
	QString msg;

	if (delaySecTotal) {
		if (delaySecStarting && !delaySecStopping) {
			msg = QTStr("Basic.StatusBar.DelayStartingIn");
			msg = msg.arg(QString::number(delaySecStarting));

		} else if (!delaySecStarting && delaySecStopping) {
			msg = QTStr("Basic.StatusBar.DelayStoppingIn");
			msg = msg.arg(QString::number(delaySecStopping));

		} else if (delaySecStarting && delaySecStopping) {
			msg = QTStr("Basic.StatusBar.DelayStartingStoppingIn");
			msg = msg.arg(QString::number(delaySecStopping),
						  QString::number(delaySecStarting));
		} else {
			msg = QTStr("Basic.StatusBar.Delay");
			msg = msg.arg(QString::number(delaySecTotal));
		}
	}

	ui->delayInfoLabel->setText(msg);
}

#define BITRATE_UPDATE_SECONDS 2

void XLStatusBar::UpdateBandwidth()
{
	if (!streamOutput)
		return;

	if (++bitrateUpdateSeconds < BITRATE_UPDATE_SECONDS)
		return;

	uint64_t bytesSent     = obs_output_get_total_bytes(streamOutput);
	uint64_t bytesSentTime = os_gettime_ns();

	if (bytesSent < lastBytesSent)
		bytesSent = 0;
	if (bytesSent == 0)
		lastBytesSent = 0;

	uint64_t bitsBetween   = (bytesSent - lastBytesSent) * 8;

	double timePassed = double(bytesSentTime - lastBytesSentTime) /
						1000000000.0;

	double kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

	QString text;
	text += QString("kb/s: ") +
			QString::number(kbitsPerSec, 'f', 0);

	ui->networkSpeedLabel->setText(text);
	ui->networkSpeedLabel->setMinimumWidth(ui->networkSpeedLabel->width());

	lastBytesSent        = bytesSent;
	lastBytesSentTime    = bytesSentTime;
	bitrateUpdateSeconds = 0;
}

void XLStatusBar::UpdateCPUUsage()
{
	OBSBasic *main = dynamic_cast<OBSBasic*>(parent());
	if (!main)
		return;

	QString text;
	text += QString("CPU: ") +
			QString::number(main->GetCPUUsage(), 'f', 1) + QString("%, ") +
			QString::number(obs_get_active_fps(), 'f', 2) + QString(" fps");

	ui->cpuLabel->setText(text);
	ui->cpuLabel->setMinimumWidth(ui->cpuLabel->width());
}

void XLStatusBar::UpdateStreamTime()
{
	totalStreamSeconds++;

	int seconds      = totalStreamSeconds % 60;
	int totalMinutes = totalStreamSeconds / 60;
	int minutes      = totalMinutes % 60;
	int hours        = totalMinutes / 60;

	QString text;
	text.sprintf("LIVE: %02d:%02d:%02d", hours, minutes, seconds);
	ui->liveTimeLabel->setText(text);
	ui->liveTimeLabel->setMinimumWidth(ui->liveTimeLabel->width());

	if (reconnectTimeout > 0) {
		QString msg = QTStr("Basic.StatusBar.Reconnecting")
			.arg(QString::number(retries),
				 QString::number(reconnectTimeout));
		showMessage(msg);
		reconnectTimeout--;

	} else if (retries > 0) {
		QString msg = QTStr("Basic.StatusBar.AttemptingReconnect");
		showMessage(msg.arg(QString::number(retries)));
	}

	if (delaySecStopping > 0 || delaySecStarting > 0) {
		if (delaySecStopping > 0)
			--delaySecStopping;
		if (delaySecStarting > 0)
			--delaySecStarting;
		UpdateDelayMsg();
	}
}

void XLStatusBar::UpdateRecordTime()
{
	totalRecordSeconds++;

	int seconds      = totalRecordSeconds % 60;
	int totalMinutes = totalRecordSeconds / 60;
	int minutes      = totalMinutes % 60;
	int hours        = totalMinutes / 60;

	QString text;
	text.sprintf("REC: %02d:%02d:%02d", hours, minutes, seconds);
	ui->recordTimeLabel->setText(text);
	ui->recordTimeLabel->setMinimumWidth(ui->recordTimeLabel->width());
}

void XLStatusBar::UpdateDroppedFrames()
{
	if (!streamOutput)
		return;

	int totalDropped = obs_output_get_frames_dropped(streamOutput);
	int totalFrames  = obs_output_get_total_frames(streamOutput);
	double percent   = (double)totalDropped / (double)totalFrames * 100.0;

	if (!totalFrames)
		return;

	QString text = QTStr("DroppedFrames");
	text = text.arg(QString::number(totalDropped),
					QString::number(percent, 'f', 1));
	ui->dropFrameLabel->setText(text);
	ui->dropFrameLabel->setMinimumWidth(ui->dropFrameLabel->width());

	/* ----------------------------------- *
	 * calculate congestion color          */

	float congestion = obs_output_get_congestion(streamOutput);
	float avgCongestion = (congestion + lastCongestion) * 0.5f;
	if (avgCongestion < congestion)
		avgCongestion = congestion;
	if (avgCongestion > 1.0f)
		avgCongestion = 1.0f;

	if (avgCongestion < EPSILON) {
		ui->networkSpeedIconLabel->setPixmap(greenPixmap);
	} else if (fabsf(avgCongestion - 1.0f) < EPSILON) {
		ui->networkSpeedIconLabel->setPixmap(redPixmap);
	} else {
		QPixmap pixmap(20, 20);

		float red = avgCongestion * 2.0f;
		if (red > 1.0f) red = 1.0f;
		red *= 255.0;

		float green = (1.0f - avgCongestion) * 2.0f;
		if (green > 1.0f) green = 1.0f;
		green *= 255.0;

		pixmap.fill(QColor(int(red), int(green), 0));
		ui->networkSpeedIconLabel->setPixmap(pixmap);
	}

	lastCongestion = congestion;
}

void XLStatusBar::OBSOutputReconnect(void *data, calldata_t *params)
{
	XLStatusBar *statusBar =
		reinterpret_cast<XLStatusBar*>(data);

	int seconds = (int)calldata_int(params, "timeout_sec");
	QMetaObject::invokeMethod(statusBar, "Reconnect", Q_ARG(int, seconds));
	UNUSED_PARAMETER(params);
}

void XLStatusBar::OBSOutputReconnectSuccess(void *data, calldata_t *params)
{
	XLStatusBar *statusBar =
		reinterpret_cast<XLStatusBar*>(data);

	QMetaObject::invokeMethod(statusBar, "ReconnectSuccess");
	UNUSED_PARAMETER(params);
}

void XLStatusBar::Reconnect(int seconds)
{
	OBSBasic *main = dynamic_cast<OBSBasic*>(parent());

	if (!retries)
		main->SysTrayNotify(
			QTStr("Basic.SystemTray.Message.Reconnecting"),
			QSystemTrayIcon::Warning);

	reconnectTimeout = seconds;

	if (streamOutput) {
		delaySecTotal = obs_output_get_active_delay(streamOutput);
		UpdateDelayMsg();

		retries++;
	}
}

void XLStatusBar::ReconnectClear()
{
	retries              = 0;
	reconnectTimeout     = 0;
	bitrateUpdateSeconds = -1;
	lastBytesSent        = 0;
	lastBytesSentTime    = os_gettime_ns();
	delaySecTotal        = 0;
	UpdateDelayMsg();
}

void XLStatusBar::ReconnectSuccess()
{
	OBSBasic *main = dynamic_cast<OBSBasic*>(parent());

	QString msg = QTStr("Basic.StatusBar.ReconnectSuccessful");
	showMessage(msg, 4000);
	main->SysTrayNotify(msg, QSystemTrayIcon::Information);
	ReconnectClear();

	if (streamOutput) {
		delaySecTotal = obs_output_get_active_delay(streamOutput);
		UpdateDelayMsg();
	}
}

void XLStatusBar::showMessage(QString msg, int timeout) {
	ui->msgLabel->setText(msg);
	if(timeout > 0) {
		QTimer* t = new QTimer(this);
		t->setSingleShot(true);
		connect(t, &QTimer::timeout, this, &XLStatusBar::clearMessage);
		t->start(timeout);
	}
}

void XLStatusBar::clearMessage() {
	ui->msgLabel->setText("");
}

void XLStatusBar::UpdateStatusBar()
{
	OBSBasic *main = dynamic_cast<OBSBasic*>(parent());

	UpdateBandwidth();

	if (streamOutput)
		UpdateStreamTime();

	if (recordOutput)
		UpdateRecordTime();

	UpdateDroppedFrames();

	int skipped = video_output_get_skipped_frames(obs_get_video());
	int total   = video_output_get_total_frames(obs_get_video());

	skipped -= startSkippedFrameCount;
	total   -= startTotalFrameCount;

	int diff = skipped - lastSkippedFrameCount;
	double percentage = double(skipped) / double(total) * 100.0;

	if (diff > 10 && percentage >= 0.1f) {
		showMessage(QTStr("HighResourceUsage"), 4000);
		if (!main->isVisible() && overloadedNotify) {
			main->SysTrayNotify(QTStr("HighResourceUsage"),
								QSystemTrayIcon::Warning);
			overloadedNotify = false;
		}
	}

	lastSkippedFrameCount = skipped;
}

void XLStatusBar::StreamDelayStarting(int sec)
{
	OBSBasic *main = dynamic_cast<OBSBasic*>(parent());
	if (!main || !main->outputHandler)
		return;

	streamOutput = main->outputHandler->streamOutput;

	delaySecTotal = delaySecStarting = sec;
	UpdateDelayMsg();
	Activate();
}

void XLStatusBar::StreamDelayStopping(int sec)
{
	delaySecTotal = delaySecStopping = sec;
	UpdateDelayMsg();
}

void XLStatusBar::StreamStarted(obs_output_t *output)
{
	streamOutput = output;

	signal_handler_connect(obs_output_get_signal_handler(streamOutput),
						   "reconnect", OBSOutputReconnect, this);
	signal_handler_connect(obs_output_get_signal_handler(streamOutput),
						   "reconnect_success", OBSOutputReconnectSuccess, this);

	retries           = 0;
	lastBytesSent     = 0;
	lastBytesSentTime = os_gettime_ns();
	Activate();
}

void XLStatusBar::StreamStopped()
{
	if (streamOutput) {
		signal_handler_disconnect(
			obs_output_get_signal_handler(streamOutput),
			"reconnect", OBSOutputReconnect, this);
		signal_handler_disconnect(
			obs_output_get_signal_handler(streamOutput),
			"reconnect_success", OBSOutputReconnectSuccess,
			this);

		ReconnectClear();
		streamOutput = Q_NULLPTR;
		clearMessage();
		Deactivate();
	}
}

void XLStatusBar::RecordingStarted(obs_output_t *output) {
	recordOutput = output;
	Activate();
}

void XLStatusBar::RecordingStopped() {
	recordOutput = Q_NULLPTR;
	Deactivate();
}
