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

#include "xl-combo-box.hpp"
#include <QStyledItemDelegate>

XLComboBox::XLComboBox(QWidget *parent) :
	QComboBox(parent) {
	// WORKAROUND for stylesheet
	// QCompleter sets a custom QAbstractItemDelegate on it's model and unfortunately this custom item
	// delegate does not inherit QStyledItemDelegate but simply QItemDelegate (and then overrides the
	// paint method to show the selected state).
	setItemDelegate(new QStyledItemDelegate());
}