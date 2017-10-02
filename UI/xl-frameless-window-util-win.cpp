#include "xl-frameless-window-util-osx.hpp"

#ifdef Q_OS_WIN

void XLFramelessWindowUtil::setupUI(QMainWindow* w) {
    w->setWindowFlags(Qt::FramelessWindowHint);
}

#endif // #ifdef Q_OS_WIN