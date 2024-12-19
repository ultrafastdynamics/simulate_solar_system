#pragma once

#include <vector>
#include <string>
#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QDir>
#include <fstream>
#include "qt_square_frame.hpp"

class QCustomPlot;
class QColor;
class QCPAxis;
class QCPCurve;
class QCPItemPixmap;
class QCPItemText;
class QCPRange;
class Object;

class QTPlotframe : public QTSquareFrame
{

    Q_OBJECT

    public : 

    QTPlotframe(QWidget *parent);

    QCustomPlot* getPlot();

    const std::vector<QCustomPlot*>& getVelocityPlots();

    void update(const std::vector<double>& state);

    void addObjects(const std::vector<Object>& objects);

    void setPlanetRadius(double radius);

    void setStepsPerTrajectoryUpdate(unsigned int steps);

    void useDarkmode(bool darkmode = false);

    void clear();

    void clearTrajectories();

    void showTrajectories(bool show);

    void showTrajectoryLines(bool show);

    double getKeplerArea();

    const std::vector<std::string>& getNames();

    void followObject();
    void setFollowIndex(int index);
    void toggleGrid();
    void setGridVisible();

    QColor background_color_{QColor(255, 255, 255)};

    private slots:

    void onXRangeChanged(const QCPRange& range); //prevents infinite zoom in
    void onYRangeChanged(const QCPRange& range);
    void zoom(double factor);
    void move(double x_factor, double y_factor);

    protected :

    QCPItemPixmap* setUpPixmapItem(const QPixmap& icon, const std::string& name);

    QString formatPixmapName(const std::string& name);

    void setAxisColor(QCPAxis* axis);

    void setAllColors();

    // QString getIconDir();
    QDir getIconDir();

    QCustomPlot* plot_{};
    QCPItemText* day_label_;
    std::vector<std::string> names_{};
    std::vector<QPixmap> icons_{};
    std::vector<QCPItemPixmap*> pixmaps_items_{};
    QVector<double> time_;
    QVector<double> area_per_timestep_;
    std::vector<QVector<double>> x_{};
    std::vector<QVector<double>> y_{};
    std::vector<QVector<double>> v_x_;
    std::vector<QVector<double>> v_y_;
    std::vector<QCPCurve*> curves_;
    std::vector<QCustomPlot*> velocity_plots_{};
    std::ifstream results_;
    double initial_dimension_{1e8};
    double planet_radius_{8e6};
    QColor v_x_color_{Qt::blue};
    QColor v_y_color_{Qt::red};
    QColor tick_color_{Qt::black};
    int follow_index_{-1};
};
