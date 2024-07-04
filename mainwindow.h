#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QProgressBar>
#include <QFile>
#include <QTextStream>
#include <vector>
#include <QCoreApplication>
#include "Electrode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    vector<int> initialFreq;
    vector<int> finalFreq;
    int startingOverallBaseline=0;
    int endOverallBaseline=0;

private slots:

    //  navigate the menu in the up direction
    void on_upButton_clicked();

    // navigate the menu in the down direction
    void on_downButton_clicked();

    // countdown timer
    void countdown();

    // update the progress bar
    void updateBar();

    //when the session ends succesfully the date and time is saved
    void saveSession();

    // when patient selects NEW SESSION, SESSION LOG, or TIME AND DATE in the menu
    void newSession();
    void sessionLog();
    void timeAndDate();

    // flashes the green light
    void flashGreenLight();

    // flash the red light
    void flashRedLight();

    // turns off the green light. This function is used whenever the green light stops flashing, because
    // we can't guarantee that the green light will stop flashing on the gray color. If it stops flashing when green,
    // that's a problem.
    void turnOffGreenLight();

    void turnOffRedLight();

    // the device beeps
    void beep();

    // deplete the battery icon every second
    void depleteBattery();

    // fully charges the battery
    void resetBattery();

    // stops the session prematurely
    void stopSession();

    // turn device off and on
    void turnDeviceOff();
    void turnDeviceOn();

    // checks every EEG site (checkboxes on the form) to see if they are all active or not
    // basically just looking for if at least one checkbox is unchecked
    void checkForSensorDisconnect();

    // sets all EEG site connections (checkboxes) at once, for quick on/off testing
    void setAllSensors(bool state);

    // checkbox state change checker
    // all of these are identical. basically whenever a checkbox is changed in any way, it'll run checkForSensorDisconnect.
    void on_checkBoxSensor_1_stateChanged(int arg1);
    void on_checkBoxSensor_2_stateChanged(int arg1);
    void on_checkBoxSensor_3_stateChanged(int arg1);
    void on_checkBoxSensor_4_stateChanged(int arg1);
    void on_checkBoxSensor_5_stateChanged(int arg1);
    void on_checkBoxSensor_6_stateChanged(int arg1);
    void on_checkBoxSensor_7_stateChanged(int arg1);
    void on_checkBoxSensor_8_stateChanged(int arg1);
    void on_checkBoxSensor_9_stateChanged(int arg1);
    void on_checkBoxSensor_10_stateChanged(int arg1);
    void on_checkBoxSensor_11_stateChanged(int arg1);
    void on_checkBoxSensor_12_stateChanged(int arg1);
    void on_checkBoxSensor_13_stateChanged(int arg1);
    void on_checkBoxSensor_14_stateChanged(int arg1);
    void on_checkBoxSensor_15_stateChanged(int arg1);
    void on_checkBoxSensor_16_stateChanged(int arg1);
    void on_checkBoxSensor_17_stateChanged(int arg1);
    void on_checkBoxSensor_18_stateChanged(int arg1);
    void on_checkBoxSensor_19_stateChanged(int arg1);
    void on_checkBoxSensor_20_stateChanged(int arg1);
    void on_checkBoxSensor_21_stateChanged(int arg1);
    void on_pushButtonSensorsOn_clicked();

    void on_pushButtonSensorsOff_clicked();

    void on_playButton_clicked();
    void on_pauseButton_clicked();
    void on_stopButton_clicked();
    void on_menuButton_clicked();

    void on_powerButton_clicked();

    void on_replaceBatteryButton_clicked();

    void on_dateTimeEdit_dateTimeChanged(const QDateTime &newDateTime);

    void on_testGamma_clicked();

    void on_testBeta_clicked();

    void on_testAlpha_clicked();

    void on_testTheta_clicked();

    void on_testDelta_clicked();

private:
    Ui::MainWindow *ui;

    // timer for the total countdown in a session
    QTimer *sessionTimer;

    // timer that makes the green light flash
    QTimer *lightTimer;

    // timer that depletes the battery every second
    QTimer *batteryTimer;

    // timer that waits 5 minutes during a disconnect to terminate the session
    QTimer *fiveminuteTimer;

    // timer that makes the red light flash whenever there's an EEG disconnect
    QTimer *disconnectRedLightTimer;

    // timer that makes device beep when there's an EEG disconnect
    QTimer *beepTimer;

    // the date and time for sessions
    QDateTime dateTime;

    // countdown in seconds
    int secsRemaining;

    // the time it will take to calculate average dominant frequency in seconds
    int calculateSecs;

    // the time it will take to apply treatment in seconds
    int treatmentSecs;

    // the amount of seconds the timer started with
    int totalTime;

    // the total amount of battery seconds
    int totalBattery;

    // the remaining seconds in battery
    int remainingBattery;

    // true if the green light is on, off if not
    bool greenLightOn;

    // buttons act differently depending on what the device is doing
    // 'm' for menu, the play button will select an option from the menu, all other buttons do not work (except power)
    // 's' for session, the play button will continue a paused session, the pause button will pause a session, the menu button does not work, the stop button stops the session
    // 'f' for finished session, the play, pause, and stop button do not work, the menu and power buttons work
    // 'o' for off, all buttons have no functionality (except power)
    char deviceState;

    // "true" if not all EEG sites are connected (session should stop / pause / not start)
    // "false" if every EEG site is connected (session can continue)
    bool eegDisconnect;

    // red light flashing (eeg disconnect)
    bool redLightOn;

    Electrode** electrodes;

    int testMax;
    int testMin;
};
#endif // MAINWINDOW_H
