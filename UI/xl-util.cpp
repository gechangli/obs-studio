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