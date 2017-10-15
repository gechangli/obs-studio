#include "xl-volume-control.hpp"
#include "qt-wrappers.hpp"
#include "obs-app.hpp"
#include <QSlider>
#include <QPainter>
#include <QCheckBox>

using namespace std;

void XLVolControl::OBSVolumeChanged(void *data, float db)
{
	Q_UNUSED(db);
	XLVolControl *volControl = static_cast<XLVolControl*>(data);

	QMetaObject::invokeMethod(volControl, "VolumeChanged");
}

void XLVolControl::OBSVolumeMuted(void *data, calldata_t *calldata)
{
	XLVolControl *volControl = static_cast<XLVolControl*>(data);
	bool muted = calldata_bool(calldata, "muted");

	QMetaObject::invokeMethod(volControl, "VolumeMuted",
							  Q_ARG(bool, muted));
}

void XLVolControl::VolumeChanged()
{
	slider->blockSignals(true);
	slider->setValue((int) (obs_fader_get_deflection(obs_fader) * 100.0f));
	slider->blockSignals(false);
}

void XLVolControl::VolumeMuted(bool muted)
{
	if (mute->isChecked() != muted)
		mute->setChecked(muted);
}

void XLVolControl::SetMuted(bool checked)
{
	obs_source_set_muted(source, checked);
}

void XLVolControl::SliderChanged(int vol)
{
	obs_fader_set_deflection(obs_fader, float(vol) * 0.01f);
}

XLVolControl::XLVolControl(OBSSource source_, QCheckBox* muteCheckBox, QSlider* volumeSlider) :
	source(source_),
	mute(muteCheckBox),
	slider(volumeSlider),
	obs_fader(obs_fader_create(OBS_FADER_CUBIC))
{
	QString sourceName = obs_source_get_name(source);
	slider->setMinimum(0);
	slider->setMaximum(100);

	bool muted = obs_source_muted(source);
	mute->setChecked(muted);
	mute->setAccessibleName(
		QTStr("VolControl.Mute").arg(sourceName));

	obs_fader_add_callback(obs_fader, OBSVolumeChanged, this);

	signal_handler_connect(obs_source_get_signal_handler(source),
						   "mute", OBSVolumeMuted, this);

	QWidget::connect(slider, SIGNAL(valueChanged(int)),
					 this, SLOT(SliderChanged(int)));
	QWidget::connect(mute, SIGNAL(clicked(bool)),
					 this, SLOT(SetMuted(bool)));

	obs_fader_attach_source(obs_fader, source);

	/* Call volume changed once to init the slider position and label */
	VolumeChanged();
}

XLVolControl::~XLVolControl()
{
	obs_fader_remove_callback(obs_fader, OBSVolumeChanged, this);

	signal_handler_disconnect(obs_source_get_signal_handler(source),
							  "mute", OBSVolumeMuted, this);

	obs_fader_destroy(obs_fader);
}