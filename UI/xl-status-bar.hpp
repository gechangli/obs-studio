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
#include <memory>
#include <QPointer>
#include <QTimer>
#include <util/platform.h>
#include <obs.h>
#include "ui_XLStatusBar.h"

class XLStatusBar : public QWidget {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLStatusBar> ui;
	obs_output_t *streamOutput = nullptr;
	obs_output_t *recordOutput = nullptr;
	bool active = false;
	bool overloadedNotify = true;

	int retries = 0;
	int totalStreamSeconds = 0;
	int totalRecordSeconds = 0;

	int reconnectTimeout = 0;

	int delaySecTotal = 0;
	int delaySecStarting = 0;
	int delaySecStopping = 0;

	int startSkippedFrameCount = 0;
	int startTotalFrameCount = 0;
	int lastSkippedFrameCount = 0;

	int      bitrateUpdateSeconds = 0;
	uint64_t lastBytesSent = 0;
	uint64_t lastBytesSentTime = 0;

	QPixmap transparentPixmap;
	QPixmap greenPixmap;
	QPixmap grayPixmap;
	QPixmap redPixmap;

	float lastCongestion = 0.0f;

	QPointer<QTimer> refreshTimer;

	obs_output_t *GetOutput();

	void Activate();
	void Deactivate();

	void UpdateDelayMsg();
	void UpdateBandwidth();
	void UpdateStreamTime();
	void UpdateRecordTime();
	void UpdateDroppedFrames();

	void showMessage(QString msg, int timeout = 0);
	void clearMessage();

	static void OBSOutputReconnect(void *data, calldata_t *params);
	static void OBSOutputReconnectSuccess(void *data, calldata_t *params);

private slots:
	void Reconnect(int seconds);
	void ReconnectSuccess();
	void UpdateStatusBar();
	void UpdateCPUUsage();

protected:
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

public:
	XLStatusBar(QWidget* parent = Q_NULLPTR);
	virtual ~XLStatusBar();

	void StreamDelayStarting(int sec);
	void StreamDelayStopping(int sec);
	void StreamStarted(obs_output_t *output);
	void StreamStopped();
	void RecordingStarted(obs_output_t *output);
	void RecordingStopped();
	void ReconnectClear();
};