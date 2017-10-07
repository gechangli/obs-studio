#include "xl-frameless-window-util.hpp"

#ifdef Q_OS_WIN

void XLFramelessWindowUtil::setupUI(QMainWindow* w) {
    w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
}

#endif // #ifdef Q_OS_WIN