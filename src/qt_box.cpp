#include "qt_box.hpp"
#include <cmath>
#include <QEvent>

QTBox::QTBox(double start, uint decimals, double min, double max, QWidget* parent)
    : QScienceSpinBox(parent)
{
    installEventFilter(this);
    setValue(start);
    setDecimals(decimals);
    setMinimum(min);
    setMaximum(max);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setSizePolicy(QSizePolicy());
}

bool QTBox::eventFilter(QObject*, QEvent* event)
{
    return event->type() == QEvent::Wheel;
}
