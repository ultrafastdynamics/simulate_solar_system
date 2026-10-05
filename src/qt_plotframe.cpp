#include "qt_plotframe.hpp"
#include "object.hpp"
#include "qcustomplot.h"
#include <QFont>
#include <cassert>
#include <ctype.h>
#include <sstream>

static std::array<QColor, 4> light_colors{Qt::white, Qt::cyan, Qt::magenta, Qt::yellow};
int seconds_per_day = 86400;

QTPlotframe::QTPlotframe(QWidget *parent)
    : QFrame(parent)
{
    plot_ = new QCustomPlot(parent);
    plot_->installEventFilter(this);
    plot_->axisRect()->setBackground(QPixmap(QDir(getIconDir().canonicalPath() + "/space.jpg").canonicalPath()));
    for(auto& rect:  plot_->axisRects()){
        rect->setAutoMargins(QCP::MarginSide::msNone);
        rect->setMargins(QMargins(60, 30, 0, 30));
    }

    plot_->legend->setVisible(false);
    day_label_ = new QCPItemText(plot_);
    day_label_->setPositionAlignment(Qt::AlignLeft|Qt::AlignTop);
    day_label_->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
    day_label_->position->setType(QCPItemPosition::ptAxisRectRatio);
    day_label_->position->setCoords(0.02, 0.02); // place position at left/top of axis rect
    day_label_->setText("Day 0");
    day_label_->setPadding(QMargins(1, 1, 3, 3)); // spacing around text
    
    const QFont label_font(font().family(), 16);
    day_label_->setFont(label_font); // make font a bit larger
    day_label_->setPen(QPen(Qt::black)); // show black border around text
    day_label_->setBrush(QBrush(Qt::white)); // background color

    day_label_->setSelectedFont(label_font);
    day_label_->setSelectedColor(Qt::black);
    //day_label_->setSelectedPen(QPen(QColor(60, 100, 160)));      // gedämpftes Blau, gleiche Strichstärke
    day_label_->setSelectedBrush(QBrush(QColor(255,150,60))); 

    // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
    plot_->xAxis2->setVisible(true);
    plot_->xAxis2->setTickLabels(false);
    plot_->yAxis2->setVisible(true);
    plot_->yAxis2->setTickLabels(false);
    // prevent infinite zoom:
    connect(plot_->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onXRangeChanged(QCPRange)));
    connect(plot_->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onYRangeChanged(QCPRange)));
    
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    plot_->xAxis->setRange(-initial_dimension_, initial_dimension_);
    plot_->yAxis->setRange(-initial_dimension_, initial_dimension_);
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectItems);
    setAllColors();
    connect(new QShortcut(Qt::Key_Plus, this), &QShortcut::activated, this, [&](){zoom(0.9);});
    connect(new QShortcut(Qt::Key_Minus, this), &QShortcut::activated, this, [&](){zoom(1.1);});
    connect(new QShortcut(Qt::Key_Up, this), &QShortcut::activated, this, [&](){move(0, 0.1);});
    connect(new QShortcut(Qt::Key_Down, this), &QShortcut::activated, this, [&](){move(0, -0.1);});
    connect(new QShortcut(Qt::Key_Left, this), &QShortcut::activated, this, [&](){move(-0.1, 0);});
    connect(new QShortcut(Qt::Key_Right, this), &QShortcut::activated, this, [&](){move(0.1, 0);});
    connect(plot_, &QCustomPlot::mousePress, this, [this](QMouseEvent*) { auto_fit_ = false; });
    connect(plot_, &QCustomPlot::mouseWheel, this, [this](QWheelEvent*) { auto_fit_ = false; });

}

void QTPlotframe::addObjects(const std::vector<Object>& objects)
{
    if(objects.empty())
    {
        return;
    }
    x_ = std::vector<QVector<double>>(objects.size());
    y_ = x_;
    v_x_ = x_;
    v_y_ = x_;
    traj_x_ = x_;
    traj_y_ = x_;
    traj_end_.clear();
    std::vector<double> x,y;
    for(size_t i = 0; i < objects.size(); i++)
    {
        auto pixmap = QPixmap(formatPixmapName(objects[i].getName()));
        if(pixmap.isNull())
        {
            pixmap = QPixmap(formatPixmapName("default"));
        }
        icons_.push_back(pixmap);
        names_.push_back(objects[i].getName());
        x.push_back(objects[i].getX() / 1000.);
        y.push_back(objects[i].getY() / 1000.);
        pixmaps_items_.push_back(setUpPixmapItem(icons_[i], names_[i]));

        line_curves_.push_back(new QCPCurve(plot_->xAxis, plot_->yAxis));
        line_curves_[i]->setPen(QPen(light_colors[i % light_colors.size()]));
        
        velocity_plots_.push_back(new QCustomPlot());
        velocity_plots_[i]->addGraph();
        velocity_plots_[i]->addGraph();
        velocity_plots_[i]->yAxis->ticker()->setTickCount(3);
        velocity_plots_[i]->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        velocity_plots_[i]->axisRect()->setRangeZoom(Qt::Horizontal);
        velocity_plots_[i]->axisRect()->setRangeDrag(Qt::Horizontal);
    }
    setAllColors();

    const double new_dimension = 1.2 * std::max(
        std::max(*std::max_element(x.begin(), x.end()), std::fabs(*std::min_element(x.begin(), x.end()))),
        std::max(*std::max_element(y.begin(), y.end()), std::fabs(*std::min_element(y.begin(), y.end()))));

    // Nur neu einpassen, wenn sich die Ausdehnung der Objekte wirklich geändert hat.
    // So bleibt dein Zoom erhalten, wenn du z.B. nur einen Namen tippst oder neu startest.
    if(new_dimension > 0. && std::fabs(new_dimension - initial_dimension_) > 1e-9 * new_dimension)
    {
        initial_dimension_ = new_dimension;
        auto_fit_ = true;
        keepAspectRatio(plot_->size());
    }
}

QString QTPlotframe::formatPixmapName(const std::string& name)
{
    size_t length = name.length();
    // while(isdigit(name[length - 1]))
    while(length > 0 and (isdigit(name[length - 1]) or (name[length - 1] == '_') or isspace(name[length - 1])))
    {
        length--;
    }
    std::string _name = name.substr(0, length);
    std::transform(_name.begin(), _name.end(), _name.begin(), [](unsigned char c){ return std::tolower(c); });
    QString filename = getIconDir().canonicalPath() + "/" + QString::fromStdString(_name + ".png");
    return QDir(filename).canonicalPath();
}

void QTPlotframe::update(const std::vector<double>& state, const std::vector<double>& trajectory)
{
    const bool append = !curves_dirty_;
    view_index_ = -1; 
    curves_dirty_ = false;

    time_.push_back(state[0] / seconds_per_day);
    area_per_timestep_.push_back(state[1] / 1e6);

    const double r = imageRadius();

    const size_t n = pixmaps_items_.size();
    const size_t samples = n ? trajectory.size() / (2 * n) : 0;

    for(size_t i = 0; i < n; i++)
    {
        double x = state[2 + 4*i] / 1000;
        double y = state[3 + 4*i] / 1000;
        placeImage(pixmaps_items_[i], x, y, r);
        x_[i].push_back(x);
        y_[i].push_back(y);
        v_x_[i].push_back(state[4 + 4*i] / 1000);
        v_y_[i].push_back(state[5 + 4*i] / 1000);
        velocity_plots_[i]->graph(0)->setData(time_, v_x_[i], true);
        velocity_plots_[i]->graph(1)->setData(time_, v_y_[i], true);
        double min = std::min(*std::min_element(v_x_[i].begin(), v_x_[i].end()), *std::min_element(v_y_[i].begin(), v_y_[i].end()));
        double max = std::max(*std::max_element(v_x_[i].begin(), v_x_[i].end()), *std::max_element(v_y_[i].begin(), v_y_[i].end()));
        velocity_plots_[i]->yAxis->setRange(min, max);
        velocity_plots_[i]->xAxis->setRange(0, time_.last());
        velocity_plots_[i]->replot();

        QVector<double> new_x, new_y;
        new_x.reserve(static_cast<int>(samples));
        new_y.reserve(static_cast<int>(samples));
        for(size_t s = 0; s < samples; s++)
        {
            new_x.push_back(trajectory[s * 2 * n + 2 * i]     / 1000.);
            new_y.push_back(trajectory[s * 2 * n + 2 * i + 1] / 1000.);
        }
        traj_x_[i] += new_x;
        traj_y_[i] += new_y;
        if(append)
        {
            line_curves_[i]->addData(new_x, new_y);   // nur anhängen statt alles neu kopieren
        }
        else
        {
            line_curves_[i]->setData(traj_x_[i], traj_y_[i]);   // nach Historienansicht neu aufbauen
        }
    }
    traj_end_.push_back(n ? traj_x_[0].size() : 0);

    long day = static_cast<long>(state[0]) / seconds_per_day;
    day_label_->setText(QString::fromStdString("Day " + std::to_string(day)));
    if (follow_index_ != -1){
        followObject();
    }
    plot_->replot();
}

double QTPlotframe::getKeplerArea()
{
    return area_per_timestep_.back();
}

void QTPlotframe::showTrajectoryLines(bool show)
{
    auto linestyle = QCPCurve::lsNone;
    if(show)
    {
        linestyle = QCPCurve::lsLine;
    }
    for(auto* curve : line_curves_)
    {
        curve->setLineStyle(linestyle);
    }
}

void QTPlotframe::useDarkmode(bool darkmode)
{
    if(darkmode)
    {
        background_color_ = QColor(169,169,169);
        tick_color_ = Qt::black;
        v_x_color_ = Qt::cyan;
        v_y_color_ = Qt::magenta;
    }
    else
    {
        background_color_ = QColor(196, 223, 230);
        tick_color_ = Qt::black;
        v_x_color_ = Qt::blue;
        v_y_color_ = Qt::red;
    }
    setAllColors();
}

void QTPlotframe::clear()
{
    view_index_ = -1; 
    icons_.clear();
    names_.clear();
    x_.clear();
    y_.clear();
    v_x_.clear();
    v_y_.clear();
    line_curves_.clear();
    traj_x_.clear();
    traj_y_.clear();
    traj_end_.clear();
    curves_dirty_ = false;
    for(int i = plot_->itemCount() - 1; i >= 0; --i)
    {
        if(plot_->item(i) != day_label_)   // day_label_ muss bleiben
        {
            plot_->removeItem(plot_->item(i));
        }
    }
    plot_->clearPlottables();
    for(auto* plot : velocity_plots_)
    {
        plot->clearPlottables();
    }
    velocity_plots_.clear();
    x_.clear();
    y_.clear();
    pixmaps_items_.clear();
    area_per_timestep_.clear();
    time_.clear();
}

void QTPlotframe::onXRangeChanged(const QCPRange& range)
{
    QCPRange boundedRange = range;
    const double min_range = 4. * min_radius_;

    if(boundedRange.size() < min_range)
    {
        boundedRange.lower = boundedRange.center() - min_range / 2.;
        boundedRange.upper = boundedRange.lower + min_range;
    }
    plot_->xAxis->setRange(boundedRange);
    plot_->xAxis2->setRange(boundedRange);
    updateImageSizes();
}

void QTPlotframe::onYRangeChanged(const QCPRange& range)
{
    QCPRange boundedRange = range;
    const double min_range = 4. * min_radius_;

    if(boundedRange.size() < min_range)
    {
        boundedRange.lower = boundedRange.center() - min_range / 2.;
        boundedRange.upper = boundedRange.lower + min_range;
    }
    plot_->yAxis->setRange(boundedRange);
    plot_->yAxis2->setRange(boundedRange);
    updateImageSizes();
}

QCustomPlot* QTPlotframe::getPlot()
{
    return plot_;
}

const std::vector<QCustomPlot*>& QTPlotframe::getVelocityPlots()
{
    return velocity_plots_;
}

void QTPlotframe::setAllColors()
{
    for(auto* plot : velocity_plots_)
    {
        plot->setBackground(background_color_);
        plot->graph(0)->setPen(QPen(v_x_color_));
        plot->graph(1)->setPen(QPen(v_y_color_));
        setAxisColor(plot->xAxis);
        setAxisColor(plot->yAxis);
    }
    plot_->setBackground(background_color_);
    setAxisColor(plot_->xAxis);
    setAxisColor(plot_->yAxis);
}

void QTPlotframe::setAxisColor(QCPAxis* axis)
{
    axis->setTickLabelColor(tick_color_);
    axis->setBasePen(QPen(tick_color_));
    axis->setLabelColor(tick_color_);
    axis->setTickPen(QPen(tick_color_));
    axis->setSubTickPen(QPen(tick_color_));
}

QCPItemPixmap* QTPlotframe::setUpPixmapItem(const QPixmap& icon, const std::string& name)
{
    auto *pixitem = new QCPItemPixmap(plot_);
    pixitem->setPixmap(icon);
    pixitem->setScaled(true, Qt::IgnoreAspectRatio);
    pixitem->setSelectable(false);
    // pixitem->setScaled(true);

    // add the text label at the top:
    QCPItemText *label = new QCPItemText(plot_);
    label->setColor(Qt::white);
    label->position->setParentAnchor(pixitem->top);
    label->position->setCoords(0, -10); // move 10 pixels to the top from bracket center anchor
    label->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    label->setText(QString::fromStdString(name));
    label->setFont(QFont(font().family(), 14));
    label->setSelectable(false);
    return pixitem;
}

QDir QTPlotframe::getIconDir()
{
    QDir directory = QCoreApplication::applicationDirPath();
    directory.cd("../icons/");
    return directory;
}

const std::vector<std::string>& QTPlotframe::getNames()
{
    return names_;
}

void QTPlotframe::followObject()   // ersetzt die Version von vorhin
{
    const int idx = currentIndex();
    const double half_width  = plot_->xAxis->range().size() / 2.;
    const double half_height = plot_->yAxis->range().size() / 2.;
    const double cx = x_[follow_index_].at(idx);
    const double cy = y_[follow_index_].at(idx);
    plot_->xAxis->setRange(cx - half_width,  cx + half_width);
    plot_->yAxis->setRange(cy - half_height, cy + half_height);
}

void QTPlotframe::setFollowIndex(int index)
{
    follow_index_ = index;
}

void QTPlotframe::toggleGrid()
{
    bool visible = not plot_->xAxis->grid()->visible();
    plot_->xAxis->grid()->setVisible(visible);
    plot_->yAxis->grid()->setVisible(visible);
}

void QTPlotframe::setGridVisible()
{
    plot_->xAxis->grid()->setVisible(true);
    plot_->yAxis->grid()->setVisible(true);
}

void QTPlotframe::zoom(double factor)
{
    auto_fit_ = false;
    double x_center = plot_->xAxis->range().center();
    double x_size = plot_->xAxis->range().size() * factor;
    plot_->xAxis->setRange(x_center-x_size / 2.0, x_center+x_size / 2.0);

    double y_center = plot_->yAxis->range().center();
    double y_size = plot_->yAxis->range().size() * factor;
    plot_->yAxis->setRange(y_center-y_size / 2.0, y_center+y_size / 2.0);

    plot_->replot();
}

void QTPlotframe::move(double x_factor, double y_factor)
{
    auto_fit_ = false;
    double x_shift = plot_->xAxis->range().size() * x_factor;
    double y_shift = plot_->yAxis->range().size() * y_factor;
    plot_->xAxis->setRange(plot_->xAxis->range().lower + x_shift, plot_->xAxis->range().upper + x_shift);
    plot_->yAxis->setRange(plot_->yAxis->range().lower + y_shift, plot_->yAxis->range().upper + y_shift);
    plot_->replot();
}

bool QTPlotframe::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == plot_)
    {
        if(event->type() == QEvent::Resize)
        {
            keepAspectRatio(static_cast<QResizeEvent*>(event)->size());
        }
        else if(event->type() == QEvent::Wheel && day_label_->selected())
        {
            auto* wheel = static_cast<QWheelEvent*>(event);
            wheel_accum_ += wheel->angleDelta().y();
            const int notches = static_cast<int>(wheel_accum_ / 120.);  // Touchpads liefern kleine Deltas
            wheel_accum_ -= notches * 120.;
            if(notches != 0)
            {
                const int factor = (wheel->modifiers() & Qt::ControlModifier) ? 10 : 1;
                scrollHistory(notches * factor);   // Rad nach oben = Vergangenheit
            }
            return true;   // verhindert den Zoom von QCustomPlot
        }
    }
    return QFrame::eventFilter(obj, event);
}

void QTPlotframe::keepAspectRatio(const QSize& plot_size)
{
    // Der Filter läuft VOR dem resizeEvent von QCustomPlot, daher wird die
    // Größe des Achsenrechtecks aus Plotgröße und festen Margins berechnet.
    const QMargins m = plot_->axisRect()->margins();
    const double w = plot_size.width()  - m.left() - m.right();
    const double h = plot_size.height() - m.top()  - m.bottom();
    if(w <= 0 || h <= 0)
    {
        return;
    }

    double cx = 0., cy = 0., units_per_pixel;
    if(auto_fit_ || last_inner_size_.isEmpty())
    {
        if(initial_dimension_ <= 0.)
        {
            return;
        }
        // Gesamtansicht [-d, d] passt in das kleinere Maß, das andere wird erweitert
        units_per_pixel = 2. * initial_dimension_ / std::min(w, h);
    }
    else
    {
        // Maßstab beibehalten, Ausschnitt um seinen Mittelpunkt anpassen
        const QCPRange xr = plot_->xAxis->range();
        const QCPRange yr = plot_->yAxis->range();
        cx = xr.center();
        cy = yr.center();
        units_per_pixel = std::max(xr.size() / last_inner_size_.width(),
                                   yr.size() / last_inner_size_.height());
    }

    const double x_half = units_per_pixel * w / 2.;
    const double y_half = units_per_pixel * h / 2.;
    plot_->xAxis->setRange(cx - x_half, cx + x_half);
    plot_->yAxis->setRange(cy - y_half, cy + y_half);
    last_inner_size_ = QSizeF(w, h);
    // xAxis2/yAxis2 werden über onX/YRangeChanged mitgezogen
}

int QTPlotframe::currentIndex() const
{
    return view_index_ >= 0 ? view_index_ : static_cast<int>(time_.size()) - 1;
}

void QTPlotframe::scrollHistory(int steps_back)
{
    if(time_.size() < 2)
    {
        return;
    }
    const int current = currentIndex();
    const int target = std::clamp(current - steps_back, 0, static_cast<int>(time_.size()) - 1);
    if(target == current)
    {
        return;   // z.B. live und weiter "nach vorn" gescrollt: nichts tun, nicht pausieren
    }
    view_index_ = target;
    emit historyScrolled();
    showHistoryState(target);
}

void QTPlotframe::showHistoryState(int index)
{
    const double r = imageRadius();
    for(size_t i = 0; i < pixmaps_items_.size(); i++)
    {
        const double x = x_[i].at(index);
        const double y = y_[i].at(index);
        placeImage(pixmaps_items_[i], x, y, r);

        const int end = traj_end_.at(index);
        line_curves_[i]->setData(traj_x_[i].mid(0, end), traj_y_[i].mid(0, end));

        const QVector<double> t = time_.mid(0, index + 1);
        velocity_plots_[i]->graph(0)->setData(t, v_x_[i].mid(0, index + 1), true);
        velocity_plots_[i]->graph(1)->setData(t, v_y_[i].mid(0, index + 1), true);
        velocity_plots_[i]->xAxis->setRange(0, time_.at(index));
        velocity_plots_[i]->replot();
    }
    day_label_->setText(QString("Day %1").arg(static_cast<long>(time_.at(index))));
    if(follow_index_ != -1)
    {
        followObject();
    }
    curves_dirty_ = true;
    plot_->replot();
}

void QTPlotframe::showObjects(const std::vector<Object>& objects)
{
    const double r = imageRadius();
    for(size_t i = 0; i < pixmaps_items_.size() && i < objects.size(); i++)
    {
        const double x = objects[i].getX() / 1000.;
        const double y = objects[i].getY() / 1000.;
        placeImage(pixmaps_items_[i], x, y, r);
    }
    day_label_->setText("Day 0");
    plot_->replot();
}
double QTPlotframe::imageRadius() const
{
    // Seitenverhältnis ist 1:1 (keepAspectRatio), der kleinere Bereich entspricht dem kleineren Bildschirmmaß
    const double view = std::min(plot_->xAxis->range().size(), plot_->yAxis->range().size());
    return std::max(min_radius_, std::min(image_fraction_ * view, max_radius_));
}

void QTPlotframe::placeImage(QCPItemPixmap* item, double x, double y, double r)
{
    item->topLeft->setCoords(x - r, y + r);
    item->bottomRight->setCoords(x + r, y - r);
}

void QTPlotframe::updateImageSizes()
{
    const double r = imageRadius();
    for(auto* item : pixmaps_items_)
    {
        // Mittelpunkt aus den bisherigen Ecken, die Position selbst bleibt unverändert
        const QPointF c = (item->topLeft->coords() + item->bottomRight->coords()) / 2.;
        placeImage(item, c.x(), c.y(), r);
    }
}