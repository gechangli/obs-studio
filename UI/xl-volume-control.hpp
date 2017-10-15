#pragma once

#include <obs.hpp>
#include <QWidget>
#include <QSharedPointer>
#include <QTimer>
#include <QMutex>
#include <QList>

class QSlider;
class QCheckBox;

class XLVolControl : public QWidget {
	Q_OBJECT

private:
	OBSSource source;
	QSlider* slider;
	QCheckBox* mute;
	obs_fader_t* obs_fader;

	static void OBSVolumeChanged(void *param, float db);
	static void OBSVolumeMuted(void *data, calldata_t *calldata);

private slots:
	void VolumeChanged();
	void VolumeMuted(bool muted);

	void SetMuted(bool checked);
	void SliderChanged(int vol);

signals:
	void ConfigClicked();

public:
	XLVolControl(OBSSource source, QCheckBox* muteCheckBox, QSlider* volumeSlider);
	~XLVolControl();

	inline obs_source_t *GetSource() const {return source;}
};
