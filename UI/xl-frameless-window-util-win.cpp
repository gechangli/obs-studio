#include "xl-frameless-window-util.hpp"

#ifdef Q_OS_WIN

void XLFramelessWindowUtil::setupUI(QMainWindow* w) {
	// update flag
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint | Qt::Window);
}

#endif // #ifdef Q_OS_WIN