#ifndef HVCONTROLWIDGET_H
#define HVCONTROLWIDGET_H

#include <QWidget>
#include <QTimer>
#include <string>
#include <miil/log.h>

class HVController;

namespace Ui {
    class HVControlWidget;
}

class HVControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HVControlWidget(
            HVController * hv_controller,
            const std::string & default_name = "/dev/ttyS0",
            bool open_log = false,
            QWidget * parent = 0);
    ~HVControlWidget();

signals:
    void voltage_point(double voltage, int channel_number);
    void current_point(double voltage, int channel_number);

private:
    Ui::HVControlWidget *ui;
    HVController *hvController;
    QTimer timer_update_stats;
    int updateTimeMS;
    bool ready;
    void updatePushButtons();
    Log current_log;

private slots:
    void updateStats();
    void on_pushButton_connect_clicked();
    void on_pushButton_set1_clicked();
    void on_pushButton_set2_clicked();
    void on_pushButton_rampDown1_clicked();
    void on_pushButton_rampDown2_clicked();
    void on_pushButton_stop_clicked();

protected:
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
};

#endif // HVCONTROLWIDGET_H
