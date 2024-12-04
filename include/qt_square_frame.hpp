#pragma once

#include <QFrame>
#include <QResizeEvent>

class QTSquareFrame : public QFrame {
public:
    QTSquareFrame(QWidget *parent = nullptr) : QFrame(parent) {
        setFrameRect(parent->frameGeometry());
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QFrame::resizeEvent(event);
        auto newSize = std::min(event->size().width(), event->size().height());
        setFrameRect(QRect(frameRect().topLeft(), QSize(newSize, newSize))); // Make the frame a square
    }
};
