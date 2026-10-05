#pragma once

#include <vector>
#include <string>
#include <QWidget>
#include <QFrame>
#include <QPixmap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QDir>
#include <fstream>

class QCustomPlot;
class QColor;
class QCPAxis;
class QCPCurve;
class QCPItemPixmap;
class QCPItemText;
class QCPRange;
class Object;

class QTPlotframe : public QFrame
{

    Q_OBJECT

    public : 

    QTPlotframe(QWidget *parent);

    QCustomPlot* getPlot();

    const std::vector<QCustomPlot*>& getVelocityPlots();

    void update(const std::vector<double>& state, const std::vector<double>& trajectory);

    void addObjects(const std::vector<Object>& objects);

    void setStepsPerTrajectoryUpdate(unsigned int steps);

    void useDarkmode(bool darkmode = false);

    void clear();

    void clearTrajectories();

    void showTrajectoryLines(bool show);

    double getKeplerArea();

    const std::vector<std::string>& getNames();

    void followObject();
    void setFollowIndex(int index);
    void toggleGrid();
    void setGridVisible();
    void showObjects(const std::vector<Object>& objects);

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

    bool eventFilter(QObject* obj, QEvent* event) override;

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
    std::vector<QCPCurve*> line_curves_;
    std::vector<QVector<double>> traj_x_, traj_y_;
    std::vector<int> traj_end_;
    bool curves_dirty_ = false;
    std::vector<QCustomPlot*> velocity_plots_{};
    std::ifstream results_;
    double initial_dimension_{1e8};
    QColor v_x_color_{Qt::blue};
    QColor v_y_color_{Qt::red};
    QColor tick_color_{Qt::black};
    int follow_index_{-1};

    private:
    void keepAspectRatio(const QSize& plot_size);
    int  currentIndex() const;            // angezeigter Zeitindex
    void scrollHistory(int steps_back);
    void showHistoryState(int index);

    int    view_index_  = -1;             // -1 = live (neuester Zustand)
    double wheel_accum_ = 0.;
    bool   auto_fit_ = true;     // Gesamtansicht folgt der Fenstergröße
    QSizeF last_inner_size_;     // Größe des Achsenrechtecks beim letzten Anpassen
    
    double imageRadius() const;
    void updateImageSizes();
    void placeImage(QCPItemPixmap* item, double x, double y, double r);

    const double image_fraction_ = 0.03;   // Bildradius als Anteil des kleineren sichtbaren Achsenbereichs
    const double min_radius_     = 1e4;    // km, Untergrenze
    const double max_radius_     = 5e7;    // km, Untergrenze
    
    signals:
    void historyScrolled();   // Nutzer scrollt gerade durch die Vergangenheit
};
