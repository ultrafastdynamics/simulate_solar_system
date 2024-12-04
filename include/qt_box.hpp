#pragma once

#include "qscience_spin_box.hpp"

class QEvent;
class QObject;
class QWidget;

class QTBox : public QScienceSpinBox
{    
    Q_OBJECT

    public:
    QTBox(double start, uint decimals, double min, double max, QWidget* parent = nullptr);

    private:
    bool eventFilter(QObject *obj, QEvent* event);
};
