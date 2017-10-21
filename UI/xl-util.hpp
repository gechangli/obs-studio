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

#include <QString>
#include <QColor>
#include <QPixmap>

class QListWidgetItem;
class QListWidget;

class XLUtil {
public:
	// file
	static QString getFileContent(QString path);
	static QString getDataFileContent(QString path);

	// string
	static QString stringByDeletingLastPathComponent(QString path);
	static QString stringByDeletingPathExtension(QString path);
	static QString lastPathComponent(QString path);

	// pixmap
	static QPixmap createCircle(int radius, QColor color);

	// qss
	static QString loadQss(QString path, QString paramName = "Default");
	static QString getQssPathByName(QString name);

	// list
	static QListWidgetItem* takeListItem(QListWidget *widget, int row);
	static void deleteListItem(QListWidget *widget, QListWidgetItem *item);
	static void clearListItems(QListWidget *widget);
};