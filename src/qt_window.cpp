#include "qt_window.hpp"
#include "simulation/euler.hpp"
#include "simulation/rk4.hpp"
#include "qt_plotframe.hpp"
#include "qt_box.hpp"
#include "qt_object_box.hpp"
#include "object.hpp"
#include "qcustomplot.h"
#include "qscience_spin_box.hpp"
#include "ioparser.hpp"

#include <QGridLayout>
#include <QToolBar>
#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QFrame>
#include <QString>
#include <QTextStream>
#include <QComboBox>
#include <QPushButton>
#include <QLayoutItem>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QStringList>
#include <QTableWidgetItem>
#include <QShortcut>
#include <cmath>
#include <iostream>

#define STR_EQUAL 0

/***************************************
 * QTWindow
 ***************************************/
QTWindow::QTWindow(QWidget *parent)
    : QMainWindow(parent), central_widget_(new QTCentralWidget())
{
    setWindowTitle("Physik im Weltall - AG Rethfeld, RPTU in Kaiserslautern");

    QToolBar* toolbar = addToolBar("main toolbar");
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    toolbar->setStyleSheet("color: #8B0A0A; font-weight: bold");
    QAction* reset = toolbar->addAction("Reset settings");
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolbar->addWidget(empty);
    start_ = toolbar->addAction("Start");
    toolbar->addSeparator();
    restart_ = toolbar->addAction("Restart");
    restart_->setEnabled(false);
    toolbar->addSeparator();
    //toolbar->addSeparator();
    //QAction* darkmode = toolbar->addAction("Darkmode");
    //QAction* quit = toolbar->addAction("Quit Application");
    toolbar->addWidget(new QLabel("     Simulation speed: "));
    toolbar->addWidget(central_widget_->getDaysPerUpdateBox());

    QShortcut* show_save = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
    connect(show_save, &QShortcut::activated, central_widget_, &QTCentralWidget::toggleSave);
    QShortcut* toggle_grid = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_G), this);
    connect(toggle_grid, &QShortcut::activated, central_widget_, &QTCentralWidget::toggleGrid);
    short_pause_ = new QShortcut(Qt::Key_Space, this);
    connect(short_pause_, &QShortcut::activated, this, &QTWindow::onStartPause);

    //connect(darkmode, &QAction::triggered, this, &QTWindow::toggleDarkmode);
    //toggleDarkmode();

    // connect(start_, &QAction::triggered, central_widget_, &QTCentralWidget::run);
    connect(start_, &QAction::triggered, this, &QTWindow::onStartPause);
    connect(reset, &QAction::triggered, central_widget_, &QTCentralWidget::resetSettings);
    connect(central_widget_->plotframe_, &QTPlotframe::historyScrolled, this, [this]() {
        if(sim_active_ && !is_paused_)
        {
            togglePause();   // setzt auch den Button-Text auf "Continue"
        }
    });
    connect(central_widget_, &QTCentralWidget::simulationAborted, this, [this]() {
        sim_active_ = false;
        is_paused_ = false;
        updateStartButton();
    });
    connect(restart_, &QAction::triggered, this, &QTWindow::run);

    setCentralWidget(central_widget_);
    central_widget_->setFocus();

    //resize(1040, 585);
    showMaximized();
}

QTWindow::~QTWindow()
{
    delete central_widget_;
}

void QTWindow::onStartPause()
{
    if(sim_active_)
    {
        togglePause();
    }
    else
    {
        run();
    }
}

void QTWindow::run()
{
    central_widget_->run();
    sim_active_ = true;
    is_paused_ = false;
    updateStartButton();
}

void QTWindow::togglePause()
{
    is_paused_ = !is_paused_;
    central_widget_->togglePause(is_paused_);
    updateStartButton();
}

void QTWindow::updateStartButton()
{
    if(!sim_active_)
    {
        start_->setText("Start");
    }
    else
    {
        start_->setText(is_paused_ ? "Continue" : "Pause");
    }
    restart_->setEnabled(sim_active_);
}

void QTWindow::toggleDarkmode()
{
    if(darkmode_on_)
    {
        setStyleSheet("");
        central_widget_->plotframe_->useDarkmode(false);
        darkmode_on_ = false;
        central_widget_->vel_label_->setText("Velocities (time in days, v in km/s, v_x in blue, v_y in red):");
        return;
    }
    setStyleSheet("background:darkGray; color:black");
    central_widget_->plotframe_->useDarkmode(true);
    central_widget_->vel_label_->setText("Velocities (time in days, v in km/s, v_QAction* restart_x in cyan, v_y in purple):");
    darkmode_on_ = true;
}

/***************************************
 * QTCentralWidget
 ***************************************/
QTCentralWidget::QTCentralWidget(QWidget *parent)
    : QWidget(parent), plot_update_timer_(new QTimer(this))
{

    connect(plot_update_timer_, SIGNAL(timeout()), this, SLOT(updatePlot()));

    //Settings Frame
    auto *frame1 = new QFrame(this);
    frame1->setFrameStyle(QFrame::Box); //could be removed

    QLabel* numerics_label = new QLabel("Numerics:", frame1);
    numerics_label->setStyleSheet("font-weight: bold; color: #113680");

    QLabel* timestep_label = new QLabel("Timestep (in days):", frame1);

    timestep_box_ = new MyDoubleSpinBox();
    timestep_box_->setMaximum(99999999);
    timestep_box_->setMinimum(0.0001);
    timestep_box_->setDecimals(4);
    timestep_box_->setValue(0.001);
    timestep_box_->setSingleStep(0.001);
    timestep_box_->setFixedWidth(200);

    days_per_update_box_ = new MyDoubleSpinBox();
    days_per_update_box_->setMaximum(999);
    days_per_update_box_->setMinimum(0.1);
    days_per_update_box_->setDecimals(1);
    days_per_update_box_->setValue(35);
    days_per_update_box_->setSingleStep(1);
    days_per_update_box_->setFixedWidth(65);

    QLabel* sim_label = new QLabel("Solver:", frame1);

    sim_checkbox_ = new QComboBox(frame1);
    sim_checkbox_->addItem("Euler");
    sim_checkbox_->addItem("RK4");
    sim_checkbox_->setCurrentText("RK4");


    QFrame* line2 = new QFrame();
    line2->setGeometry(QRect(/* ... */));
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    

    QLabel* feature_label = new QLabel("Graphical settings:", frame1);
    feature_label->setStyleSheet("font-weight: bold; color: #113680");

    history_checkbox_line_ = new QCheckBox("Show object trajectories", frame1);
    history_checkbox_line_->setChecked(true);

    QLabel* follow_label = new QLabel("Follow object:", frame1);
    follow_box_ = new QComboBox(frame1);
    follow_box_->addItem(QString::fromStdString("don't follow"));
    follow_box_->setCurrentText(QString::fromStdString("don't follow"));

    QFrame* line5 = new QFrame();
    line5->setGeometry(QRect(/* ... */));
    line5->setFrameShape(QFrame::HLine);
    line5->setFrameShadow(QFrame::Sunken);
    

    QLabel* obj_label = new QLabel("Objects:", frame1);
    obj_label->setStyleSheet("font-weight: bold; color: #113680");

    QLabel* config_label = new QLabel("Load configuration:", frame1);
    std::string path = std::string(__FILE__).substr(0, std::string(__FILE__).find_last_of("/")) + "/../config";
    std::vector<std::string> files = IOParser::getFilesInDirectory(path);
    config_load_ = new QComboBox(frame1);
    config_load_->setFocusPolicy( Qt::StrongFocus );
    config_load_->installEventFilter(this);
    for (auto& file : files) {
        config_load_->addItem(QString::fromStdString(file));
    }
    config_load_->setCurrentText(QString::fromStdString("sun_earth"));
    connect(config_load_, &QComboBox::currentTextChanged, this, &QTCentralWidget::loadConfig);

    QPushButton* load_button = new QPushButton("Load"); //to prevent jumping of the save button
    load_button->setMinimumWidth(40);
    load_button->setMaximumWidth(80);
    load_button->setVisible(false);
    QSizePolicy sp_retain = load_button->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    load_button->setSizePolicy(sp_retain);

    save_label_ = new QLabel("Save configuration:", frame1);
    save_label_->setVisible(false);
    save_name_ = new QLineEdit(frame1);
    save_name_->setMaxLength(30);
    save_name_->setFixedWidth(200);
    save_name_->setVisible(false);

    save_button_ = new QPushButton("Save");
    save_button_->setMinimumWidth(40);
    save_button_->setMaximumWidth(80);
    save_button_->setVisible(false);
    connect(save_button_, &QPushButton::clicked, this, &QTCentralWidget::saveConfig);

    QLabel* remember_label = new QLabel("Changes here directly start a new simulation!", frame1);

    object_area_ = new QTObjectArea(frame1);
    //QFrame* line4 = new QFrame();
    //line4->setGeometry(QRect(/* ... */));
    //line4->setFrameShape(QFrame::HLine);
    //line4->setFrameShadow(QFrame::Sunken);

    vel_label_ = new QLabel("Velocities (time in days, v in km/s, v_x in blue, v_y in red):", frame1);
    vel_label_->setStyleSheet("font-weight: bold; color: #113680");

    QScrollArea* vel_box = new QScrollArea(frame1);
    vel_box->setStyleSheet("background: #ffffff");
    auto* vel_frame_ = new QFrame(frame1);
    vel_grid_ = new QGridLayout(vel_frame_);
    vel_box->setWidget(vel_frame_);
    vel_box->setWidgetResizable(true);

    auto *left_grid_ = new QGridLayout(frame1);
    // left_grid_->setHorizontalSpacing(0);

    uint i=0;
    left_grid_->addWidget(numerics_label, i++, 0, 1, 2);
    left_grid_->addWidget(timestep_label, i, 0, 1, 1);
    left_grid_->addWidget(timestep_box_, i++, 1, 1,  1);
    left_grid_->addWidget(sim_label, i, 0, 1, 1);
    left_grid_->addWidget(sim_checkbox_, i++, 1, 1, 1);
    left_grid_->addWidget(line2, i++, 0, 1, 5);
    left_grid_->addWidget(feature_label, i++, 0, 1, 1);
    left_grid_->addWidget(history_checkbox_line_, i++, 1, 1, 1);
    left_grid_->addWidget(follow_label, i, 0, 1, 1);
    left_grid_->addWidget(follow_box_, i++, 1, 1, 1);
    left_grid_->addWidget(line5, i++, 0, 1, 5);
    left_grid_->addWidget(obj_label, i++, 0, 1, 1);
    left_grid_->addWidget(config_label, i, 0, 1, 1);
    left_grid_->addWidget(config_load_, i, 1, 1, 1);
    left_grid_->addWidget(load_button, i++, 2, 1, 1);
    left_grid_->addWidget(save_label_, i, 0, 1, 1);
    left_grid_->addWidget(save_name_, i, 1, 1, 1);
    left_grid_->addWidget(save_button_, i++, 2, 1, 1);
    left_grid_->addWidget(remember_label, i++, 1, 1, 4);
    left_grid_->addWidget(object_area_, i++, 0, 1, 5);
    //left_grid_->addWidget(line4, i++, 0, 1, 2);
    left_grid_->addWidget(vel_label_, i++, 0, 1, 5);
    left_grid_->addWidget(vel_box, i++, 0, 1, 5);

    //Plot Frame
    plotframe_ = new QTPlotframe(this);
    auto *plot = plotframe_->getPlot();

    auto* right_grid_ = new QGridLayout(plotframe_);
    right_grid_->addWidget(plot, 0, 0, 1, 1);

    auto* frame_grid = new QGridLayout(this);
    auto* splitter = new QSplitter(this);
    splitter->addWidget(frame1);
    splitter->addWidget(plotframe_);
    splitter->setSizes(QList<int>({INT_MAX,INT_MAX}));
    frame_grid->addWidget(splitter, 0, 0, 1, 1);
    //frame_grid->addWidget(frame1, 0, 0, 1, 7);
    //frame_grid->addWidget(plotframe_, 0, 7, 1, 8);
    setLayout(frame_grid);

    preview_timer_ = new QTimer(this);
    preview_timer_->setSingleShot(true);
    connect(preview_timer_, &QTimer::timeout, this, &QTCentralWidget::showPreview);
    connect(object_area_, &QTObjectArea::objectsChanged, this, &QTCentralWidget::onObjectsChanged);
    preview_timer_->start(0);   // Vorschau schon für die Startkonfiguration
    connect(timestep_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QTCentralWidget::onObjectsChanged);
    connect(sim_checkbox_, &QComboBox::currentTextChanged, this, &QTCentralWidget::onObjectsChanged);
}

QTCentralWidget::~QTCentralWidget()
{
}

bool QTCentralWidget::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::Wheel)
    {
        QComboBox* combo = qobject_cast<QComboBox*>(obj);
        if(combo && !combo->hasFocus())
            return true;
    }

    return false;
}

void QTCentralWidget::updateFollowObjects(){
    follow_box_->clear();
    follow_box_->addItem(QString::fromStdString("don't follow"));
    follow_box_->setCurrentIndex(0);
    for (auto& name : plotframe_->getNames()) {
        follow_box_->addItem(QString::fromStdString(name));
    }
}

void QTCentralWidget::toggleSave()
{
    if(save_label_->isVisible())
    {
        save_label_->setVisible(false);
        save_name_->setVisible(false);
        save_button_->setVisible(false);
    }
    else
    {
        save_label_->setVisible(true);
        save_name_->setVisible(true);
        save_button_->setVisible(true);
    }
}

void QTCentralWidget::toggleGrid(){
    plotframe_->toggleGrid();
}

QDoubleSpinBox* QTCentralWidget::getDaysPerUpdateBox()
{
    return days_per_update_box_;
}

void QTCentralWidget::run()
{
    generateSimulation();
    plot_update_timer_->start(ms_per_update_);
    updateFollowObjects();
}

void QTCentralWidget::updatePlot()
{
    simulation_->setDaysBetweenSaves(days_per_update_box_->value()*ms_per_update_/1000.);
    simulation_->runTillNextSave();
    plotframe_->setFollowIndex(follow_box_->currentIndex()-1); // "don't follow" is at index 0
    plotframe_->update(simulation_->getState(), simulation_->getTrajectory());
    plotframe_->showTrajectoryLines(history_checkbox_line_->isChecked());
    updateTimer();
}

void QTCentralWidget::stop()
{
    if(plot_update_timer_)
    {
        plot_update_timer_->stop();
    }
}

void QTCentralWidget::proceed()
{
    if(plot_update_timer_)
    {
        plot_update_timer_->start(ms_per_update_);
    }
}

void QTCentralWidget::togglePause(bool is_paused)
{
    is_paused ? stop() : proceed();
}

void QTCentralWidget::resetSettings()
{
    timestep_box_->setValue(0.001);
    days_per_update_box_->setValue(35.);
    sim_checkbox_->setCurrentText("RK4");
    config_load_->setCurrentText("sun_earth");
    history_checkbox_line_->setChecked(true);
    object_area_->resetObjects();
    follow_box_->setCurrentIndex(0);
    plotframe_->setGridVisible();
}

void QTCentralWidget::loadConfig()
{
    std::string path = std::string(__FILE__).substr(0, std::string(__FILE__).find_last_of("/")) + "/../config/"+config_load_->currentText().toStdString();
    object_area_->clearEntries();
    auto lines = IOParser::fileAsLines(path);
    for(auto& line : lines)
    {
        object_area_->createObject(line);
    }
}

void QTCentralWidget::saveConfig()
{
    QString save_name = save_name_->text();
    std::string path = std::string(__FILE__).substr(0, std::string(__FILE__).find_last_of("/")) + "/../config/"+save_name.toStdString();
    config_load_->addItem(save_name);
    object_area_->save(path);
}

void QTCentralWidget::updateTimer()
{
    plot_update_timer_->stop();
    plot_update_timer_->start(ms_per_update_);
}

std::vector<Object> QTCentralWidget::rebuildScene()
{
    auto& vel_plots = plotframe_->getVelocityPlots();
    for(uint i=0; i<vel_plots.size(); i++)
    {
        QLayoutItem* item = vel_grid_->itemAtPosition(2*i, 0);
        vel_grid_->removeItem(item);
        item->widget()->deleteLater();
        QLayoutItem* item2 = vel_grid_->itemAtPosition(2*i+1, 0);
        vel_grid_->removeItem(item2);
        item2->widget()->deleteLater();
    }
    plotframe_->clear();
    std::vector<Object> objects = object_area_->generateObjects();
    plotframe_->addObjects(objects);
    std::vector<std::string> names = plotframe_->getNames();
    for(uint i=0; i<vel_plots.size(); i++)
    {
        vel_grid_->addWidget(new QLabel(QString::fromStdString(names[i]+":")), 2*i, 0, 1, 1);
        vel_grid_->addWidget(vel_plots[i], 2*i+1, 0, 1, 1);
    }
    return objects;
}

void QTCentralWidget::generateSimulation()
{
    std::vector<Object> objects = rebuildScene();
    plotframe_->showObjects(objects);   // Objekte bleiben bis zum ersten Update sichtbar

    if(QString::compare(sim_checkbox_->currentText(), QString("RK4")) == STR_EQUAL)
    {
        simulation_ = std::make_unique<SimulationRK4>(objects);
    }
    else
    {
        simulation_ = std::make_unique<SimulationEuler>(objects);
    }
    simulation_->setTimestepInDays(timestep_box_->value());
}

void QTCentralWidget::onObjectsChanged()
{
    if(simulation_)
    {
        plot_update_timer_->stop();
        simulation_.reset();
        emit simulationAborted();
    }
    preview_timer_->start(150);
}

void QTCentralWidget::showPreview()
{
    if(simulation_)   // inzwischen wurde neu gestartet
    {
        return;
    }
    std::vector<Object> objects = rebuildScene();
    plotframe_->setFollowIndex(-1);
    plotframe_->showObjects(objects);
    updateFollowObjects();
}

/***************************************
 * QTObjectArea
 ***************************************/
QTObjectArea::QTObjectArea(QWidget* parent)
    : QTableWidget(parent)
{
    setColumnCount(7);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalHeader()->setVisible(false);
    setHorizontalHeaderLabels({"", "name", "x (km)", "y (km)", "v_x (km/s)", "v_y (km/s)", "mass (kg)"});
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QPalette p = QPalette();
    p.setColor(QPalette::Base, QColor(255, 255, 255));
    setPalette(p);

    creator_ = new QPushButton("+");
    creator_->setMinimumWidth(20);
    creator_->setMaximumWidth(60);
    connect(creator_, &QPushButton::clicked, this, &QTObjectArea::createEntry);
    
    setRowCount(current_row_+1);
    setCellWidget(current_row_, 0, creator_);
    for(int j = 1; j < columnCount(); ++j) {
        setItem(current_row_, j, new QTableWidgetItem());
        item(current_row_, j)->setFlags(Qt::NoItemFlags);
    }
    setColumnWidth(0, 30);
    
    createInitialObjects();

    int totalWidth = 0;
    for (int i = 0; i < columnCount(); ++i) {
        totalWidth += columnWidth(i);
    }

    totalWidth += verticalScrollBar()->width();
    setMaximumWidth(totalWidth);
}

void QTObjectArea::save(std::string filename) const
{
    std::ifstream file(filename);
    std::string newFilename = filename;
    int counter = 0;
    while(file.good())
    {
        counter++;
        newFilename = filename +"_"+ std::to_string(counter);
        file.close();
        file.open(newFilename);
    }
    filename = newFilename;

    std::ofstream newFile(filename);
    if(!newFile.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }

    for(auto& entry : entries_)
    {
        newFile << entry->to_string() << std::endl;
    }
    newFile.close();
}

void QTObjectArea::clearEntries()
{
    for(uint i = ids_.size(); i > 0; i--)
    {
        removeEntry(ids_[i - 1]);
    }
}

void QTObjectArea::resetObjects()
{
    clearEntries();
    createInitialObjects();
}

void QTObjectArea::createObject(std::string content)
{
    createEntry();
    entries_.back()->fill_all(content);
}

void QTObjectArea::createInitialObjects()
{
    createEntry();
    createEntry();
    entries_[0]->fill("Sun");
    entries_[1]->fill("Earth");
}

void QTObjectArea::createEntry()
{
    uint id = getID();
    ids_.push_back(id);

    auto* object_box = new QTObjectBox();
    entries_.push_back(object_box);

    QPushButton* del_button = new QPushButton("x");
    del_button->setMinimumWidth(20);
    del_button->setMaximumWidth(60);
    connect(del_button, &QPushButton::clicked, [=] {this->removeEntry(id);});
    del_buttons_.push_back(del_button);

    insertRow(current_row_);
    uint i=0;
    setCellWidget(current_row_, i++, del_button);

    for(auto type : {QTObjectBox::NAME, QTObjectBox::X, QTObjectBox::Y,
                     QTObjectBox::V_X, QTObjectBox::V_Y, QTObjectBox::MASS})
    {
        QWidget* box = object_box->getBox(type);
        setCellWidget(current_row_, i++, box);
        watchEdits(box);
    }
    current_row_++;
    emit objectsChanged();
}

std::vector<Object> QTObjectArea::generateObjects()
{
    std::vector<Object> objects;
    for(auto* entry: entries_)
    {
        Object* obj = entry->generateObject();
        if(obj)
        {
            bool unique_pos = true;
            for (auto& other : objects)
            {
                if(other.getPosition() == obj->getPosition())
                {
                    unique_pos = false;
                }
            }
            if(unique_pos) objects.push_back(*obj);
        }
    }
    return objects;
}

void QTObjectArea::removeEntry(uint id)
{
    uint row_idx = std::distance(ids_.begin(), std::find(ids_.begin(), ids_.end(), id));
    removeRow(row_idx);

    ids_.erase(ids_.begin()+row_idx);
    del_buttons_[row_idx]->deleteLater();
    del_buttons_.erase(del_buttons_.begin()+row_idx);
    entries_[row_idx]->deleteLater();
    entries_.erase(entries_.begin()+row_idx);

    current_row_--;
    emit objectsChanged();
}

void QTObjectArea::watchEdits(QWidget* widget)
{
    if(auto* edit = qobject_cast<QLineEdit*>(widget))
    {
        connect(edit, &QLineEdit::textChanged, this, &QTObjectArea::objectsChanged);
    }
    else if(auto* spin = qobject_cast<QDoubleSpinBox*>(widget))
    {
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &QTObjectArea::objectsChanged);
    }
}