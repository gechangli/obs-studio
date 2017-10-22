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

#include "xl-add-camera-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddCameraDialog::XLAddCameraDialog(QWidget *parent) :
	QDialog (parent),
	ui(new Ui::XLAddCameraDialog) {
	// init ui
	ui->setupUi(this);

	// setup frameless ui
	XLFramelessWindowUtil::setupUI(this);

	// create title bar
	m_titleBar = new XLTitleBarSub(this);
	m_titleBar->init();
	m_titleBar->move(0, 0);
	connect(m_titleBar, &XLTitleBar::windowRequestClose, this, &QDialog::reject);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-camera-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);

	// get source id, name and create it
	char* id = "";
#ifdef Q_OS_OSX
	id = "av_capture_input";
#elif defined(Q_OS_WIN)
	id = "dshow_input";
#endif
	const char* name = obs_source_get_display_name(id);
	m_source = obs_source_create(id, name, NULL, nullptr);

	// add to scene
	OBSBasic* main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());
	OBSScene scene = main->GetCurrentScene();
	auto addSource = [](void *data, obs_scene_t *scene) {
		XLAddCameraDialog *window = static_cast<XLAddCameraDialog*>(data);
		obs_sceneitem_t* sceneitem = obs_scene_add(scene, window->getSource());
		obs_sceneitem_set_visible(sceneitem, true);
	};
	obs_enter_graphics();
	obs_scene_atomic_update(scene, addSource, this);
	obs_leave_graphics();

	obs_source_inc_showing(m_source);
	auto addDrawCallback = [this]() {
		obs_display_add_draw_callback(ui->preview->GetDisplay(),
									  XLAddCameraDialog::drawPreview, this);
	};

	enum obs_source_type type = obs_source_get_type(m_source);
	uint32_t caps = obs_source_get_output_flags(m_source);
	bool drawable_type = type == OBS_SOURCE_TYPE_INPUT ||
						 type == OBS_SOURCE_TYPE_SCENE;

	if (drawable_type && (caps & OBS_SOURCE_VIDEO) != 0) {
		connect(ui->preview, &OBSQTDisplay::DisplayCreated, addDrawCallback);
	}
}

obs_source_t* XLAddCameraDialog::getSource() {
	return m_source;
}

void XLAddCameraDialog::on_yesButton_clicked() {
	accept();
}

void XLAddCameraDialog::setWindowTitle(const QString& title) {
	m_titleBar->setWindowTitle(title);
}

void XLAddCameraDialog::getScaleAndCenterPos(
	int baseCX, int baseCY, int windowCX, int windowCY,
	int &x, int &y, float &scale) {
	double windowAspect, baseAspect;
	int newCX, newCY;

	windowAspect = double(windowCX) / double(windowCY);
	baseAspect   = double(baseCX)   / double(baseCY);

	if (windowAspect > baseAspect) {
		scale = float(windowCY) / float(baseCY);
		newCX = int(double(windowCY) * baseAspect);
		newCY = windowCY;
	} else {
		scale = float(windowCX) / float(baseCX);
		newCX = windowCX;
		newCY = int(float(windowCX) / baseAspect);
	}

	x = windowCX/2 - newCX/2;
	y = windowCY/2 - newCY/2;
}

void XLAddCameraDialog::drawPreview(void *data, uint32_t cx, uint32_t cy) {
	XLAddCameraDialog *window = static_cast<XLAddCameraDialog*>(data);
	if (!window->getSource())
		return;

	uint32_t sourceCX = max(obs_source_get_width(window->getSource()), 1u);
	uint32_t sourceCY = max(obs_source_get_height(window->getSource()), 1u);

	int   x, y;
	int   newCX, newCY;
	float scale;

	getScaleAndCenterPos(sourceCX, sourceCY, cx, cy, x, y, scale);

	newCX = int(scale * float(sourceCX));
	newCY = int(scale * float(sourceCY));

	gs_viewport_push();
	gs_projection_push();
	gs_ortho(0.0f, float(sourceCX), 0.0f, float(sourceCY),
			 -100.0f, 100.0f);
	gs_set_viewport(x, y, newCX, newCY);

	obs_source_video_render(window->getSource());

	gs_projection_pop();
	gs_viewport_pop();
}