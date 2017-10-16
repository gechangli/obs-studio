#include "xl-frameless-window-util.hpp"

#ifdef Q_OS_WIN

void XLFramelessWindowUtil::setupUI(QWidget* w) {
	// update flag
	if(dynamic_cast<QMainWindow*>(w)) {
    	w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint | Qt::Window);
    } else {
    	w->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    }
}

#endif // #ifdef Q_OS_WIN