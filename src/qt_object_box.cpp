#include "ioparser.hpp"
#include "qt_object_box.hpp"
#include "object.hpp"
#include <iostream>
#include <sstream>
#include <QHBoxLayout>
#include <QEvent>

#define STR_EQUAL 0

static QStringList planets = {"Sun", "Mercury", "Venus", "Earth", "Moon",
    "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto"};

QTObjectBox::QTObjectBox(QWidget* parent) : QFrame(parent), grid_(new QHBoxLayout(this))
{
    grid_->setMargin(0);
    grid_->setSpacing(15);

    name_->setEditable(true);
    name_->addItem("");
    name_->addItems(planets);
    name_->setCompleter(nullptr);
    name_->installEventFilter(this);
    name_->setSizePolicy(QSizePolicy());
    name_->setContentsMargins(0, 0, 0, 0);

    connect(name_, &QComboBox::currentTextChanged, this, [=](QString text) {this->fill(text);});

    grid_->addWidget(name_, 0, Qt::AlignLeft); 
    grid_->addWidget(x_, 0, Qt::AlignLeft);
    grid_->addWidget(y_, 0, Qt::AlignLeft);
    grid_->addWidget(v_x_, 0, Qt::AlignLeft);
    grid_->addWidget(v_y_, 0, Qt::AlignLeft);
    grid_->addWidget(mass_, 0, Qt::AlignLeft);
}

Object* QTObjectBox::generateObject()
{
    if(mass_->value() == 0 or name_->currentText().isEmpty())
    {
        return nullptr;
    }
    return new Object(x_->value() * 1e3, y_->value() * 1e3, v_x_->value() * 1e3, v_y_->value() * 1e3, mass_->value(), name_->currentText().toStdString());
}

std::string QTObjectBox::to_string() const
{
    std::ostringstream oss;
    std::string separator = " ";
    oss << name_->currentText().toStdString() << separator
        << x_->value() << separator << y_->value() << separator
        << v_x_->value() << separator << v_y_->value() << separator
        << mass_->value();
    return oss.str();
}

void QTObjectBox::fill(const QString& text)
{
    name_->setCurrentText(text);
    if(QString::compare(text, QString("Sun")) == STR_EQUAL){
        x_->setValue(0);
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(0);
        mass_->setValue(1.99e30);
    }
    else if(QString::compare(text, QString("Mercury")) == STR_EQUAL){
        x_->setValue(4.6e7);
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(-5.8986e1);
        mass_->setValue(3.30e23);
    }
    else if(QString::compare(text, QString("Venus")) == STR_EQUAL){
        x_->setValue(-1.0890e8); 
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(3.4803e1);
        mass_->setValue(4.87e24);
    }
    else if(QString::compare(text, QString("Earth")) == STR_EQUAL){
        x_->setValue(1.5214e8);
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(-2.929e1);
        mass_->setValue(5.97e24);
    }
    else if(QString::compare(text, QString("Moon")) == STR_EQUAL){
        x_->setValue(1.5252e8); 
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(-2.827e1);
        mass_->setValue(7.35e22);
    }
    else if(QString::compare(text, QString("Mars")) == STR_EQUAL){
        x_->setValue(2.493e8); 
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(2.1968e1);
        mass_->setValue(6.42e23);
    }
    else if(QString::compare(text, QString("Jupiter")) == STR_EQUAL){
        x_->setValue(7.506e8);
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(1.3534e1);
        mass_->setValue(1.9e27);
    }
    else if(QString::compare(text, QString("Saturn")) == STR_EQUAL){
        x_->setValue(-1.3576e9); 
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(-1.0125e1);
        mass_->setValue(5.68e26);
    }
    else if(QString::compare(text, QString("Uranus")) == STR_EQUAL){
        x_->setValue(-3.0014e9); 
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(6.4973e0);
        mass_->setValue(8.68e25);
    }
    else if(QString::compare(text, QString("Neptune")) == STR_EQUAL){
        x_->setValue(-4.4711e9); 
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(-5.4655e0);
        mass_->setValue(1.02e26);
    }
    else if(QString::compare(text, QString("Pluto")) == STR_EQUAL){
        x_->setValue(7.38e9);
        y_->setValue(0);
        v_x_->setValue(0);
        v_y_->setValue(-3.67e0);
        mass_->setValue(1.3e22);
    }
}

void QTObjectBox::fill_all(std::string content)
{
    std::vector<std::string> tokens = IOParser::split(content, ' ');
    if(tokens.size() != 6){
        std::cerr << "Line could not be parsed to object: " << content << std::endl;
        exit(1);
    }
    name_->setCurrentText(QString::fromStdString(tokens[0]));
    x_->setValue(std::stod(tokens[1]));
    y_->setValue(std::stod(tokens[2]));
    v_x_->setValue(std::stod(tokens[3]));
    v_y_->setValue(std::stod(tokens[4]));
    mass_->setValue(std::stod(tokens[5]));
}

bool QTObjectBox::eventFilter(QObject*, QEvent* event)
{
    return event->type() == QEvent::Wheel;
}

QWidget* QTObjectBox::getBox(BoxType p){
    switch(p){
        case NAME:
            return name_;
        case X:
            return x_;
        case Y:
            return y_;
        case V_X:
            return v_x_;
        case V_Y:
            return v_y_;
        case MASS:
            return mass_;
        default:
            return nullptr; 
    }
}