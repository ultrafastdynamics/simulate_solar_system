#pragma once

#include <string>
#include <QFrame>
#include <utility>

#include <QComboBox>
#include "qt_box.hpp"

// class QTBox;
class QHBoxLayout;
// class QComboBox;
class Object;

class QTObjectBox : public QFrame
{
    Q_OBJECT

    public :

    QTObjectBox(QWidget* parent = nullptr);

    void fill(const QString& name);

    void fill_all(std::string content);

    //returns nullptr if invalid object is generated
    Object* generateObject();

    //! parses the entries to a single line
    std::string to_string() const;
    
    enum BoxType{NAME, X, Y, V_X, V_Y, MASS};

    QWidget* getBox(BoxType t);

    private :

    bool eventFilter(QObject *obj, QEvent* event);

    QComboBox* name_{new QComboBox()};

    QTBox* x_{new QTBox(0, 4, -9.9999e9, 9.9999e9)};
    QTBox* y_{new QTBox(0, 4, -9.9999e9, 9.9999e9)};
    QTBox* v_x_{new QTBox(0, 4, -9.9999e9, 9.9999e9)};
    QTBox* v_y_{new QTBox(0, 4, -9.9999e9, 9.9999e9)};
    QTBox* mass_{new QTBox(1e22, 3, 0, 9.999e35)};

    QHBoxLayout* grid_{};
};
