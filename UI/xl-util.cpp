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

#include "xl-util.hpp"
#include "platform.hpp"
#include <QFile>
#include <QTextStream>
#include <QBitmap>
#include <QPainter>
#include <QListWidget>
#include "obs-app.hpp"
#include "window-basic-main.hpp"

using namespace std;

QString XLUtil::getFileContent(QString path) {
	QFile f(path);
	f.open(QIODevice::ReadOnly);
	QTextStream in(&f);
	return in.readAll();
}

QString XLUtil::getDataFileContent(QString path) {
	string jsPath;
	GetDataFilePath(path.toStdString().c_str(), jsPath);
	QFile jsFile(QString::fromStdString(jsPath));
	jsFile.open(QIODevice::ReadOnly);
	QTextStream in(&jsFile);
	return in.readAll();
}

QString XLUtil::stringByDeletingLastPathComponent(QString path) {
	int idx = path.lastIndexOf('/');
	if(idx == 0) {
		return "/";
	} else {
		return path.left(idx);
	}
}

QString XLUtil::stringByDeletingPathExtension(QString path) {
	int idx = path.lastIndexOf('.');
	if(idx != -1) {
		return path.left(idx);
	} else {
		return path;
	}
}

QString XLUtil::lastPathComponent(QString path) {
	int idx = path.lastIndexOf('/');
	if(idx == -1) {
		return path;
	} else if(path.length() > 1) {
		return path.right(path.length() - idx - 1);
	} else {
		return "/";
	}
}

QPixmap XLUtil::createCircle(int radius, QColor color) {
	// generate mask
	QSize size(radius << 1, radius << 1);
	QBitmap mask(size);
	QPainter painter(&mask);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
	painter.setBrush(QColor(0, 0, 0));
	painter.drawEllipse(QPoint(radius, radius), radius, radius);

	// generate a color square
	QPixmap src(radius << 1, radius << 1);
	src.fill(color);

	// mask square to get circle
	src.setMask(mask);
	return src;
}

QString XLUtil::getQssPathByName(QString name) {
	char userDir[512];
	name = "themes/" + name + ".qss";
	QString temp = "obs-studio/" + name;
	int ret = GetConfigPath(userDir, sizeof(userDir), temp.toStdString().c_str());
	string path;
	if (ret > 0 && QFile::exists(userDir)) {
		return QString::fromUtf8(userDir);
	} else if (GetDataFilePath(name.toStdString().c_str(), path)) {
		return QString::fromStdString(path);
	} else {
		return name;
	}
}

QString XLUtil::loadQss(QString path, QString paramName) {
	// if param name is not set, get it from qss path
	QString paramPath;
	if(paramName.isEmpty()) {
		paramPath = XLUtil::stringByDeletingPathExtension(path);
	} else {
		paramPath = XLUtil::stringByDeletingLastPathComponent(path);
		paramPath += "/" + paramName;
	}

	// append suffix
#ifdef Q_OS_WIN
	paramPath += "-win.qssp";
#elif defined(Q_OS_OSX)
	paramPath += "-osx.qssp";
#endif

	// load qss, load param, replace param, then set stylesheet
	QString qss = XLUtil::getFileContent(path);
	if(!paramPath.isEmpty()) {
		QFile f(paramPath);
		f.open(QIODevice::ReadOnly);
		QTextStream in(&f);
		while(!in.atEnd()) {
			QString line = in.readLine();
			QStringList pair = line.split("=");
			if(pair.count() == 2) {
				qss = qss.replace(pair[0], pair[1]);
			}
		}
	}
	return qss;
}

QListWidgetItem* XLUtil::takeListItem(QListWidget *widget, int row) {
	QListWidgetItem *item = widget->item(row);
	if (item) {
		delete widget->itemWidget(item);
	}
	return widget->takeItem(row);
}

void XLUtil::deleteListItem(QListWidget *widget, QListWidgetItem *item) {
	if (item) {
		delete widget->itemWidget(item);
		delete item;
	}
}

void XLUtil::clearListItems(QListWidget *widget) {
	widget->setCurrentItem(Q_NULLPTR, QItemSelectionModel::Clear);
	for (int i = 0; i < widget->count(); i++) {
		delete widget->itemWidget(widget->item(i));
	}
	widget->clear();
}

XLUtil::XLSourceType XLUtil::getSourceType(const char* id) {
	if(!strcmp(id, "decklink-input") || !strcmp(id, "xshm_input") || !strcmp(id, "v4l2_input") ||
	   !strcmp(id, "av_capture_input") || !strcmp(id, "syphon-input") || !strcmp(id, "dshow_input") ||
	   !strcmp(id, "win-ivcam")) {
		return XLS_CAMERA;
	} else if(!strcmp(id, "color_source") || !strcmp(id, "image_source") || !strcmp(id, "slideshow")) {
		return XLS_PICTURE;
	} else if(!strcmp(id, "display_capture") || !strcmp(id, "monitor_capture")) {
		return XLS_MONITOR;
	} else if(!strcmp(id, "window_capture") || !strcmp(id, "game_capture")) {
		return XLS_APP;
	} else if(!strcmp(id, "ffmpeg_source") || !strcmp(id, "vlc_source")) {
		return XLS_VIDEO;
	} else if(!strcmp(id, "text_gdiplus") || !strcmp(id, "text_ft2_source")) {
		return XLS_TEXT;
	} else {
		return XLS_UNKNOWN;
	}
}

QPixmap XLUtil::getSourceIcon(const char* id) {
	switch(getSourceType(id)) {
		case XLS_CAMERA:
			return QPixmap(":/res/images/source_camera.png");
		case XLS_PICTURE:
			return QPixmap(":/res/images/source_picture.png");
		case XLS_MONITOR:
			return QPixmap(":/res/images/source_monitor.png");
		case XLS_APP:
			return QPixmap(":/res/images/source_window.png");
		case XLS_VIDEO:
			return QPixmap(":/res/images/source_video.png");
		case XLS_TEXT:
			return QPixmap(":/res/images/source_text.png");
		default:
			return QPixmap();
	}
}

QString XLUtil::getSourceLabel(const char* id) {
	switch(getSourceType(id)) {
		case XLS_CAMERA:
			return L("Camera");
		case XLS_PICTURE:
			return L("Picture");
		case XLS_MONITOR:
			return L("Monitor");
		case XLS_APP:
			return L("App");
		case XLS_VIDEO:
			return L("Video");
		case XLS_TEXT:
			return L("Text");
		default:
			return "";
	}
}