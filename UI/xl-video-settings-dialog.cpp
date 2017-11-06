/******************************************************************************
    Copyright (C) 2013 by Hugh Bailey <obs.jim@gmail.com>

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
#include "window-basic-main.hpp"
#include "xl-video-settings-dialog.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"

using namespace std;

XLVideoSettingsDialog::XLVideoSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::XLVideoSettingsDialog) {
	ui->setupUi(this);

	// setup frameless ui
	XLFramelessWindowUtil::setupUI(this);

	// create title bar
	m_titleBar = new XLTitleBarSub(this);
	m_titleBar->init();
	m_titleBar->move(0, 0);
	connect(m_titleBar, &XLTitleBar::windowRequestClose, this, &QDialog::reject);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-video-settings-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);

	// set title
	m_titleBar->setWindowTitle(L("Video.Settings"));

	// add resolution
	ui->resolutionComboBox->addItem("1920x1080", QVariant::fromValue(QSize(1920, 1080)));
	ui->resolutionComboBox->addItem("1536x864", QVariant::fromValue(QSize(1536, 864)));
	ui->resolutionComboBox->addItem("1440x810", QVariant::fromValue(QSize(1440, 810)));
	ui->resolutionComboBox->addItem("1280x720", QVariant::fromValue(QSize(1280, 720)));
	ui->resolutionComboBox->addItem("1152x648", QVariant::fromValue(QSize(1152, 648)));
	ui->resolutionComboBox->addItem("1096x616", QVariant::fromValue(QSize(1096, 616)));
	ui->resolutionComboBox->addItem("960x540", QVariant::fromValue(QSize(960, 540)));
	ui->resolutionComboBox->addItem("852x480", QVariant::fromValue(QSize(852, 480)));
	ui->resolutionComboBox->addItem("768x432", QVariant::fromValue(QSize(768, 432)));
	ui->resolutionComboBox->addItem("696x392", QVariant::fromValue(QSize(696, 392)));

	// add fps
	ui->fpsComboBox->addItem("60", QVariant::fromValue(60));
	ui->fpsComboBox->addItem("50", QVariant::fromValue(50));
	ui->fpsComboBox->addItem("40", QVariant::fromValue(40));
	ui->fpsComboBox->addItem("30", QVariant::fromValue(30));
	ui->fpsComboBox->addItem("20", QVariant::fromValue(20));
	ui->fpsComboBox->addItem("10", QVariant::fromValue(10));

	// vbitrate
	ui->bitRateComboBox->addItem("3600", QVariant::fromValue(3800));
	ui->bitRateComboBox->addItem("3200", QVariant::fromValue(3200));
	ui->bitRateComboBox->addItem("2500", QVariant::fromValue(2500));
	ui->bitRateComboBox->addItem("1800", QVariant::fromValue(1800));
	ui->bitRateComboBox->addItem("1200", QVariant::fromValue(1200));
	ui->bitRateComboBox->addItem("700", QVariant::fromValue(700));

	// set radio button
	OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
	config_t* cfg = main->Config();
	QString profile = config_get_string(cfg, "Video", "XLVideoProfile");
	ui->superRadioButton->setChecked(profile == "super");
	ui->highRadioButton->setChecked(profile == "high");
	ui->normalRadioButton->setChecked(profile == "normal");
	ui->customRadioButton->setChecked(profile == "custom");

	// set custom
	if(ui->customRadioButton->isChecked()) {
		int fps = config_get_int(cfg, "Video", "FPSInt");
		uint32_t cx = config_get_uint(cfg, "Video", "BaseCX");
		uint32_t cy = config_get_uint(cfg, "Video", "BaseCY");
		QSize size(cx, cy);
		uint32_t vbr = config_get_uint(cfg, "SimpleOutput", "VBitrate");
		for(int i = 0; i < ui->fpsComboBox->count(); i++) {
			if(ui->fpsComboBox->itemData(i).toInt() == fps) {
				ui->fpsComboBox->setCurrentIndex(i);
				break;
			}
		}
		for(int i = 0; i < ui->bitRateComboBox->count(); i++) {
			if(ui->bitRateComboBox->itemData(i).toUInt() == vbr) {
				ui->bitRateComboBox->setCurrentIndex(i);
				break;
			}
		}
		for(int i = 0; i < ui->resolutionComboBox->count(); i++) {
			if(ui->resolutionComboBox->itemData(i).toSize() == size) {
				ui->resolutionComboBox->setCurrentIndex(i);
				break;
			}
		}
	}
}

XLVideoSettingsDialog::~XLVideoSettingsDialog() {
}

void XLVideoSettingsDialog::accept() {
	QDialog::accept();

	// change settings
	OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
	config_t* cfg = main->Config();
	if(ui->superRadioButton->isChecked()) {
		config_set_string(cfg, "Video", "XLVideoProfile", "super");
		config_set_int(cfg, "Video", "FPSType", 1);
		config_set_int(cfg, "Video", "FPSInt", 60);
		config_set_uint(cfg, "Video", "BaseCX", 1920);
		config_set_uint(cfg, "Video", "BaseCY", 1080);
		config_set_uint(cfg, "Video", "OutputCX", 1920);
		config_set_uint(cfg, "Video", "OutputCY", 1080);
		config_set_uint(cfg, "SimpleOutput", "VBitrate", 2500);
	} else if(ui->highRadioButton->isChecked()) {
		config_set_string(cfg, "Video", "XLVideoProfile", "high");
		config_set_int(cfg, "Video", "FPSType", 1);
		config_set_int(cfg, "Video", "FPSInt", 30);
		config_set_uint(cfg, "Video", "BaseCX", 1280);
		config_set_uint(cfg, "Video", "BaseCY", 720);
		config_set_uint(cfg, "Video", "OutputCX", 1280);
		config_set_uint(cfg, "Video", "OutputCY", 720);
		config_set_uint(cfg, "SimpleOutput", "VBitrate", 1200);
	} else if(ui->normalRadioButton->isChecked()) {
		config_set_string(cfg, "Video", "XLVideoProfile", "normal");
		config_set_int(cfg, "Video", "FPSType", 1);
		config_set_int(cfg, "Video", "FPSInt", 20);
		config_set_uint(cfg, "Video", "BaseCX", 960);
		config_set_uint(cfg, "Video", "BaseCY", 540);
		config_set_uint(cfg, "Video", "OutputCX", 960);
		config_set_uint(cfg, "Video", "OutputCY", 540);
		config_set_uint(cfg, "SimpleOutput", "VBitrate", 700);
	} else {
		int fps = ui->fpsComboBox->currentData().toInt();
		QSize size = ui->resolutionComboBox->currentData().toSize();
		uint32_t vbr = ui->bitRateComboBox->currentData().toUInt();
		config_set_string(cfg, "Video", "XLVideoProfile", "custom");
		config_set_int(cfg, "Video", "FPSType", 1);
		config_set_int(cfg, "Video", "FPSInt", fps);
		config_set_uint(cfg, "Video", "BaseCX", size.width());
		config_set_uint(cfg, "Video", "BaseCY", size.height());
		config_set_uint(cfg, "Video", "OutputCX", size.width());
		config_set_uint(cfg, "Video", "OutputCY", size.height());
		config_set_uint(cfg, "SimpleOutput", "VBitrate", vbr);
	}

	// save
	config_save_safe(cfg, "tmp", Q_NULLPTR);

	// reset
	main->ResetVideo();
	main->ResetOutputs();
}

void XLVideoSettingsDialog::on_yesButton_clicked() {
	accept();
}

void XLVideoSettingsDialog::on_superRadioButton_clicked(bool checked) {
	if(checked) {
		ui->highRadioButton->setChecked(false);
		ui->normalRadioButton->setChecked(false);
		ui->customRadioButton->setChecked(false);
	}
}

void XLVideoSettingsDialog::on_highRadioButton_clicked(bool checked) {
	if(checked) {
		ui->superRadioButton->setChecked(false);
		ui->normalRadioButton->setChecked(false);
		ui->customRadioButton->setChecked(false);
	}
}

void XLVideoSettingsDialog::on_normalRadioButton_clicked(bool checked) {
	if(checked) {
		ui->superRadioButton->setChecked(false);
		ui->highRadioButton->setChecked(false);
		ui->customRadioButton->setChecked(false);
	}
}

void XLVideoSettingsDialog::on_customRadioButton_clicked(bool checked) {
	if(checked) {
		ui->highRadioButton->setChecked(false);
		ui->normalRadioButton->setChecked(false);
		ui->normalRadioButton->setChecked(false);
	}
}