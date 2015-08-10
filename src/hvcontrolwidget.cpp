/*! \class     HVControlWidget
 *  \brief     GUI interface for NIMs HV Module
 */
#include <miil/hvcontrolwidget.h>
#include <miil/hvcontroller.h>
#include <ui_hvcontrolwidget.h>
#include <string>

using namespace std;

HVControlWidget::HVControlWidget(
        HVController *hv_controller,
        const std::string & default_name,
        bool open_log,
        QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HVControlWidget),
    current_log("/home/miil/","HvPowerSupplyLog_", open_log, 100000),
    updateTimeMS(1000),
    ready(false),
    hvController(hv_controller)
{
    ui->setupUi(this);
    updatePushButtons();
    ui->lineEdit_portName->setText(default_name.c_str());
    connect(&timer_update_stats, SIGNAL(timeout()), this, SLOT(updateStats()));
}

HVControlWidget::~HVControlWidget() {
    on_pushButton_stop_clicked();
    delete ui;
}

void HVControlWidget::updateStats() {
    if (!ready) {
        return;
    }
    QString status1(hvController->readStatus1().c_str());
    QString status2(hvController->readStatus2().c_str());

    ui->label_s1->setText(status1);
    ui->label_s2->setText(status2);

    int voltage1(hvController->readVoltage1());
    int voltage2(hvController->readVoltage2());
    double current1(hvController->readCurrent1());
    double current2(hvController->readCurrent2());

    QString number;
    ui->label_u1->setText(number.setNum(voltage1));
    ui->label_u2->setText(number.setNum(voltage2));
    ui->label_i1->setText(number.setNum(current1));
    ui->label_i2->setText(number.setNum(current2));

    stringstream ss;
    ss << voltage1 << " " << current1 << " "
       << voltage2 << " " << current2;

    current_log.WriteLine(ss.str());
}

void HVControlWidget::on_pushButton_connect_clicked() {
    if (hvController->isOpen()) {
        hvController->closePort();
    }

    ui->label_usbPort->setStyleSheet("");

    QString portNameHV = ui->lineEdit_portName->text();
    if (!hvController->openPort(portNameHV.toStdString())) {
        ui->label_usbPort->setStyleSheet("color: rgb(255,0,0)");
        return;
    }

    string temp;
    string vMaxStr;
    string iMaxStr;


    if (hvController->readModuleIdentifier(temp, temp, vMaxStr, iMaxStr)) {
        ui->label_Vmax->setText(QString(vMaxStr.c_str()));
        ui->label_Imax->setText(QString(iMaxStr.c_str()));
    } else {
        // HV Controller is not ready!!!
        // Do not allow further commands.
        cout << "Cannot communicate with the HVController!" << endl;
        return;
    }

    int rampSpeed1 = hvController->readRampSpeed1();
    int rampSpeed2 = hvController->readRampSpeed2();

    if (rampSpeed1 < 0 || rampSpeed2 < 0) {
        cout << "Cannot read ramp speeds!" << endl;
        return;
    }

    ui->spinBox_v1->setValue(rampSpeed1);
    ui->spinBox_v2->setValue(rampSpeed2);


    ui->spinBox_d1->setMaximum(hvController->getVMax());
    ui->spinBox_d2->setMaximum(hvController->getVMax());

    ready = true;
    updatePushButtons();
}

void HVControlWidget::on_pushButton_set1_clicked() {
    hvController->setRampSpeed1(ui->spinBox_v1->value());
    hvController->setVoltage1(ui->spinBox_d1->value());
}

void HVControlWidget::on_pushButton_set2_clicked() {
    hvController->setRampSpeed2(ui->spinBox_v2->value());
    hvController->setVoltage2(ui->spinBox_d2->value());
}

void HVControlWidget::on_pushButton_rampDown1_clicked() {
    hvController->setRampSpeed1(ui->spinBox_v1->value());
    hvController->setVoltage1(0);
    ui->spinBox_d1->setValue(0);
}

void HVControlWidget::on_pushButton_rampDown2_clicked() {
    hvController->setRampSpeed2(ui->spinBox_v2->value());
    hvController->setVoltage2(0);
    ui->spinBox_d2->setValue(0);
}

void HVControlWidget::on_pushButton_stop_clicked() {
    if (ready) {
        hvController->closePort();
    }
    ready = false;
    updatePushButtons();
}

void HVControlWidget::updatePushButtons() {
    if (!ready) {
        ui->pushButton_set1->setEnabled(false);
        ui->pushButton_set2->setEnabled(false);
        ui->pushButton_rampDown1->setEnabled(false);
        ui->pushButton_rampDown2->setEnabled(false);
        ui->pushButton_stop->setEnabled(false);
        ui->pushButton_connect->setEnabled(true);
    } else {
        ui->pushButton_set1->setEnabled(true);
        ui->pushButton_set2->setEnabled(true);
        ui->pushButton_rampDown1->setEnabled(true);
        ui->pushButton_rampDown2->setEnabled(true);
        ui->pushButton_stop->setEnabled(true);
        ui->pushButton_connect->setEnabled(false);
    }
}

void HVControlWidget::showEvent(QShowEvent *e) {
    timer_update_stats.start(updateTimeMS);
    QWidget::showEvent(e);
}

void HVControlWidget::hideEvent(QHideEvent *e)
{
    timer_update_stats.stop();
    QWidget::hideEvent(e);
}
