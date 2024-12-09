#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QString>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QTableWidget>
#include <QTimer>
#include <array>
#include <utility>
#include <vector>
#include <memory>

//forward declarations
class QComboBox;
class QFrame;
class QGridLayout;
class QPushButton;
class QLabel;
class QSpinBox;
class QAction;
class QTimer;
class QCheckBox;
class QScienceSpinBox;
class QLineEdit;
class QTCentralWidget;
class QTObjectArea;
class QTObjectBox;
class QTBox;
class QCPItemPixmap;
class QTPlotframe;
class Simulation;
class Object;
class QShortcut;

// reimplement DoubleSpinBox to select text on focus
class MyDoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT

protected:
    void focusInEvent(QFocusEvent* event) override {
        QDoubleSpinBox::focusInEvent(event); 
        QTimer::singleShot(0, this, &QDoubleSpinBox::selectAll);
    }
};

class QTWindow : public QMainWindow
{
    Q_OBJECT

    public:
    QTWindow(QWidget *parent = nullptr);

    ~QTWindow();

    private slots:
    void run();
    void togglePause();
    void toggleDarkmode();

    private:
    QTCentralWidget* central_widget_;
    bool darkmode_on_{false};
    QAction* start_;
    QAction* pause_;
    bool is_paused_{false};
    QShortcut* short_pause_;
};

class QTCentralWidget : public QWidget
{
    Q_OBJECT

    public:
    QTCentralWidget(QWidget *parent = nullptr);
    ~QTCentralWidget();

    void run();
    QDoubleSpinBox* getDaysPerUpdateBox();
    QDoubleSpinBox* getUpdatesPerSecondBox();
    QTPlotframe* plotframe_;
    QLabel* vel_label_;

    public slots:
    void updatePlot();
    void stop();
    void proceed();
    void resetSettings();
    void loadConfig();
    void saveConfig();
    void toggleSave();
    void toggleGrid();
    void togglePause(bool is_paused);

    private:
    void generateSimulation();
    void updateTimer();
    void updateFollowObjects();
    std::unique_ptr<Simulation> simulation_;
    double ms_per_update_{100};
    QTimer* plot_update_timer_{};
    QTObjectArea* object_area_;
    QCheckBox* history_checkbox_;
    QCheckBox* history_checkbox_line_;
    QScienceSpinBox* img_size_box_;
    QGridLayout* vel_grid_;
    QComboBox* sim_checkbox_;
    MyDoubleSpinBox* timestep_box_;
    QComboBox* follow_box_;
    QComboBox* config_load_;
    QLabel* save_label_;
    QLineEdit* save_name_;
    QPushButton* save_button_;
    MyDoubleSpinBox* updates_per_second_box_;
    MyDoubleSpinBox* days_per_update_box_;
    bool eventFilter(QObject*, QEvent*);
};

class QTObjectArea : public QTableWidget
{
    Q_OBJECT

    public:
    QTObjectArea(QWidget* parent = nullptr);
    std::vector<Object> generateObjects();
    void resetObjects();
    void clearEntries();
    void createObject(std::string content);
    void save(std::string filename) const;

    private slots:
    void createEntry();

    private:
    void createInitialObjects();
    void removeEntry(uint id);
    // void fillBoxes(uint id, QString text);
    static uint getID(){static uint id=0; return id++;};
    std::vector<QTObjectBox*> entries_;
    std::vector<QPushButton*> del_buttons_;
    uint current_row_{0};
    QPushButton* creator_{};
    std::vector<uint> ids_;
};
