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
    : QTSquareFrame(parent)
{
    plot_ = new QCustomPlot(parent);
    plot_->axisRect()->setBackground(QPixmap(QDir(getIconDir().canonicalPath() + "/space.jpg").canonicalPath()));
    for(auto& rect:  plot_->axisRects()){
        rect->setAutoMargins(QCP::MarginSide::msNone);
        rect->setMargins(QMargins(60, 30, 0, 30));

    }

    plot_->legend->setVisible(false);
    day_label_ = new QCPItemText(plot_);
    day_label_->setPositionAlignment(Qt::AlignLeft|Qt::AlignTop);
    day_label_->position->setType(QCPItemPosition::ptAxisRectRatio);
    day_label_->position->setCoords(0.02, 0.02); // place position at left/top of axis rect
    day_label_->setText("Day 0");
    day_label_->setFont(QFont(font().family(), 16)); // make font a bit larger
    day_label_->setPen(QPen(Qt::black)); // show black border around text
    day_label_->setBrush(QBrush(Qt::white)); // background color
    day_label_->setPadding(QMargins(1, 1, 3, 3)); // spacing around text

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
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    setAllColors();

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
        curves_.push_back(new QCPCurve(plot_->xAxis, plot_->yAxis));
        curves_[i]->setPen(QPen(light_colors[i % light_colors.size()]));
        velocity_plots_.push_back(new QCustomPlot());
        velocity_plots_[i]->addGraph();
        velocity_plots_[i]->addGraph();
        velocity_plots_[i]->yAxis->ticker()->setTickCount(3);
    }
    setAllColors();
    initial_dimension_ = 1.2 * std::max(std::max(*std::max_element(x.begin(), x.end()), std::fabs(*std::min_element(x.begin(), x.end()))),
                                        std::max(*std::max_element(y.begin(), y.end()), std::fabs(*std::min_element(y.begin(), y.end()))));
    plot_->xAxis->setRange(-initial_dimension_, initial_dimension_);
    plot_->yAxis->setRange(-initial_dimension_, initial_dimension_);
}

QString QTPlotframe::formatPixmapName(const std::string& name)
{
    size_t length = name.length();
    // while(isdigit(name[length - 1]))
    while(isdigit(name[length - 1]) or (name[length - 1] == '_') or isspace(name[length - 1]))
    {
        length--;
    }
    QString filename = getIconDir().canonicalPath() + "/" + QString::fromStdString(name.substr(0, length) + ".png");
    return QDir(filename).canonicalPath();
}

void QTPlotframe::update(const std::vector<double>& state)
{
    time_.push_back(state[0] / seconds_per_day);
    area_per_timestep_.push_back(state[1] / 1e6);
    for(size_t i = 0; i < pixmaps_items_.size(); i++)
    {
        double x = state[2 + 4*i] / 1000;
        double y = state[3 + 4*i] / 1000;
        pixmaps_items_[i]->topLeft->setCoords(x - planet_radius_, y + planet_radius_);
        pixmaps_items_[i]->bottomRight->setCoords(x + planet_radius_, y - planet_radius_);
        x_[i].push_back(x);
        y_[i].push_back(y);
        v_x_[i].push_back(state[4 + 4*i] / 1000);
        v_y_[i].push_back(state[5 + 4*i] / 1000);
        curves_[i]->setData(x_[i], y_[i]);
        velocity_plots_[i]->graph(0)->setData(time_, v_x_[i], true);
        velocity_plots_[i]->graph(1)->setData(time_, v_y_[i], true);
        double min = std::min(*std::min_element(v_x_[i].begin(), v_x_[i].end()), *std::min_element(v_y_[i].begin(), v_y_[i].end()));
        double max = std::max(*std::max_element(v_x_[i].begin(), v_x_[i].end()), *std::max_element(v_y_[i].begin(), v_y_[i].end()));
        velocity_plots_[i]->yAxis->setRange(min, max);
        velocity_plots_[i]->xAxis->setRange(0, time_.last());
        velocity_plots_[i]->replot();
    }
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

void QTPlotframe::setPlanetRadius(double radius)
{
    planet_radius_ = radius;
}

void QTPlotframe::showTrajectories(bool show)
{
    auto scatterstyle = QCPScatterStyle::ssNone;
    if(show)
    {
        scatterstyle = QCPScatterStyle::ssCircle;
    }
    for(auto* curve : curves_)
    {
        curve->setScatterStyle(scatterstyle);
    }
}

void QTPlotframe::showTrajectoryLines(bool show)
{
    auto linestyle = QCPCurve::lsNone;
    if(show)
    {
        linestyle = QCPCurve::lsLine;
    }
    for(auto* curve : curves_)
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
    icons_.clear();
    names_.clear();
    x_.clear();
    y_.clear();
    v_x_.clear();
    v_y_.clear();
    for(size_t i = 0; i < pixmaps_items_.size(); i++)
    {
        plot_->removeItem(pixmaps_items_[i]);
        plot_->removePlottable(curves_[i]);
    }
    plot_->clearPlottables();
    curves_.clear();
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
    double min_range = planet_radius_;

    if(boundedRange.size() < planet_radius_)
    {
        boundedRange.lower = boundedRange.center() - min_range / 2.;
        boundedRange.upper = boundedRange.lower + min_range;
    }
    plot_->xAxis->setRange(boundedRange);
    plot_->xAxis2->setRange(boundedRange);
}

void QTPlotframe::onYRangeChanged(const QCPRange& range)
{
    QCPRange boundedRange = range;
    double min_range = planet_radius_;

    if(boundedRange.size() < planet_radius_)
    {
        boundedRange.lower = boundedRange.center() - min_range / 2.;
        boundedRange.upper = boundedRange.lower + min_range;
    }
    plot_->yAxis->setRange(boundedRange);
    plot_->yAxis2->setRange(boundedRange);
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
    // pixitem->setScaled(true);

    // add the text label at the top:
    QCPItemText *label = new QCPItemText(plot_);
    label->setColor(Qt::white);
    label->position->setParentAnchor(pixitem->top);
    label->position->setCoords(0, -10); // move 10 pixels to the top from bracket center anchor
    label->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    label->setText(QString::fromStdString(name));
    label->setFont(QFont(font().family(), 14));
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

void QTPlotframe::followObject()
{
    QCPRange range = plot_->xAxis->range();
    double half_width = range.size() / 2.;
    plot_->xAxis->setRange(QCPRange(x_[follow_index_].last() - half_width, x_[follow_index_].last() + half_width));
    plot_->xAxis2->setRange(QCPRange(x_[follow_index_].last() - half_width, x_[follow_index_].last() + half_width));
    plot_->yAxis->setRange(QCPRange(y_[follow_index_].last() - half_width, y_[follow_index_].last() + half_width));
    plot_->yAxis2->setRange(QCPRange(y_[follow_index_].last() - half_width, y_[follow_index_].last() + half_width));
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