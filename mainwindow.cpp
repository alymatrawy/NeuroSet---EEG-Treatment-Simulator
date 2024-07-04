#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sessionTimer = new QTimer(this);
    lightTimer = new QTimer(this);
    batteryTimer = new QTimer(this);
    fiveminuteTimer = new QTimer(this);
    disconnectRedLightTimer = new QTimer(this);
    beepTimer = new QTimer(this);
    electrodes = new Electrode*[21];
    testMax = 100;
    testMin = 1;
    for (int i = 0; i < 21; i++) {
        electrodes[i] = new Electrode(i + 1);
    }

    // the date and time for sessions, automatically set to today's date
    dateTime = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(dateTime);

    // when the program starts, the device is off
    turnDeviceOff();

    // in the actual product, it takes 1 minute (60 seconds) to calculate frequency
    // when testing, the prof wants it to take 5 seconds to calculate frequnecy.
    // so change this value to 5 seconds when testing, and 60 seconds otherwise
    calculateSecs = 5;

    // treatment will always take 1 second, in both the actual product and testing
    treatmentSecs = 1;

    // The amount of time a session takes is:
    // Time for Analysis + Time for Treatment + Time for Analysis + Time for Treatment
    //     + Time for Analysis + Time for Treatment + Time for Analysis + Time for Treatment + Time for Analysis
    // = 5 x Time for Analysis + 4 x Time for Treatment
    totalTime = 5 * calculateSecs + 4 * treatmentSecs;

    // battery will run out after roughly 3 sessions and some idle time
    totalBattery = int(3.5 * totalTime);
    remainingBattery = totalBattery;

    // green light is off
    greenLightOn = false;

    // connect timer that will flash the green light
    connect(lightTimer, SIGNAL(timeout()), this, SLOT(flashGreenLight()));

    // during a session, on every timeout(), call countdown() and updateBar()
    connect(sessionTimer, SIGNAL(timeout()), this, SLOT(countdown()));
    connect(sessionTimer, SIGNAL(timeout()), this, SLOT(updateBar()));

    // deplete the battery every second
    connect(batteryTimer, SIGNAL(timeout()), this, SLOT(depleteBattery()));

    // connect the 5 minute timer (eeg disconnect timer) to a function that turns off the device
    connect(fiveminuteTimer, SIGNAL(timeout()), this, SLOT(turnDeviceOff()));

    // connect the red light flashing timer to a function that flashes the red light
    connect(disconnectRedLightTimer, SIGNAL(timeout()), this, SLOT(flashRedLight()));

    // connect the beep timer with a function that says that the device beeps
    connect(beepTimer, SIGNAL(timeout()), this, SLOT(beep()));

    // disconnect all EEG sites by default
    // it puts them "on" then "off" so the checkboxes all fire off their "changed" logic as a sort of init
    setAllSensors(true);
    setAllSensors(false);
}

MainWindow::~MainWindow()
{
    delete sessionTimer;
    delete lightTimer;
    delete batteryTimer;
    delete disconnectRedLightTimer;
    delete fiveminuteTimer;
    delete beepTimer;
    delete ui;
    for (int i = 0; i < 21; i++) {
        delete electrodes[i];
    }
    delete electrodes;
}

void MainWindow::on_upButton_clicked()
{
    // navigate the menu in the up direction, if possible
    if (ui->menuWidget->currentRow() != 0) {
        ui->menuWidget->setCurrentRow(ui->menuWidget->currentRow()-1);
    }
}
void MainWindow::on_downButton_clicked()
{
    // navigate the menu in the down direction, if possible
    if (ui->menuWidget->currentRow() != 2) {
        ui->menuWidget->setCurrentRow(ui->menuWidget->currentRow()+1);
    }
}

void MainWindow::on_playButton_clicked()
{
    // play button is a selector for menu options
    if (deviceState == 'm') {
        // hide the menu
        ui->menuWidget->setVisible(false);

        if (ui->menuWidget->currentRow() == 0) {
            newSession();
        }
        else if (ui->menuWidget->currentRow() == 1) {
            sessionLog();
        }
        else {
            timeAndDate();
        }
    }

    // play button will resume session when paused unless there's an EEG disconnect that needs to be resolved
    else if ((deviceState == 's') and !(eegDisconnect))  {
        sessionTimer->start(1000);
        fiveminuteTimer->stop();
    }
}

void MainWindow::on_pauseButton_clicked()
{
    if (deviceState == 's') {
        sessionTimer->stop();

        // if 5 minutes pass and the session does not resume, turn off the device
        // For testing purposes, this is just five seconds.
        fiveminuteTimer->start(5000);
    }
}

void MainWindow::on_stopButton_clicked()
{
    stopSession();
}

void MainWindow::stopSession(){
    fiveminuteTimer->stop();

    // cancel the session by prematurely finishing it
    if (deviceState == 's') {
        sessionTimer->stop();
        qInfo() << "The session was cancelled.";
        ui->description->setText("The session was cancelled. Please press the menu button.");
        deviceState = 'f';

        // turn off the blue light indicating contact
        ui->blueLight->setStyleSheet("background-color: gray;");

        // in case the session is cancelled while the green light is flashing
        lightTimer->stop();
        turnOffGreenLight();

        disconnectRedLightTimer->stop();
        turnOffRedLight();
        beepTimer->stop();
    }
}

void MainWindow::on_menuButton_clicked()
{
    if (deviceState == 'f') {
        // hide the session and datetime ui
        ui->sessionWidget->setVisible(false);
        ui->dateTimeEdit->setVisible(false);
        ui->sessionLog->setVisible(false);

        // show the menu again
        ui->menuWidget->setVisible(true);
        deviceState = 'm';
    }
}

void MainWindow::on_powerButton_clicked()
{
    // if device is not off, turn it off
    if (deviceState != 'o') {
        turnDeviceOff();
    }

    // else, turn the device on and go to the menu (if there's battery)
    else if (remainingBattery > 0) {
        turnDeviceOn();
    }
}

void MainWindow::turnDeviceOff() {
    // if there's a session going on, cancel it
    stopSession();
    ui->menuWidget->setVisible(false);
    ui->sessionWidget->setVisible(false);
    ui->dateTimeEdit->setVisible(false);
    ui->sessionLog->setVisible(false);

    // stop the battery depletion
    batteryTimer->stop();
    deviceState = 'o';

    // turn the battery indicator off
    ui->battery->setStyleSheet("QProgressBar::chunk {border: 0px;	background-color: gray;} QProgressBar {border: 0px; background-color: gray;}");
    ui->battery->setTextVisible(false);

    // battery can only be replaced when device is off
    ui->replaceBatteryButton->setEnabled(true);
}

void MainWindow::turnDeviceOn() {
    ui->menuWidget->setVisible(true);
    batteryTimer->start(1000);
    deviceState = 'm';

    // turn the battery indicator on
    if (remainingBattery * 100 / totalBattery <= 10) {
        ui->battery->setStyleSheet("QProgressBar::chunk {border: 2px solid black; background-color: red;}");
    }
    else {
        ui->battery->setStyleSheet("QProgressBar::chunk {border: 2px solid black;	background-color: lightgreen;}");
    }
    ui->battery->setTextVisible(true);

    // battery can only be replaced when device is off
    ui->replaceBatteryButton->setDisabled(true);
}

// insert behaviour for NEW SESSION in this function. Ex. the use case for new session
void MainWindow::newSession() {
    // show the timer and progress bar
    ui->sessionWidget->setVisible(true);

    // set the amount of seconds remaining
    secsRemaining = totalTime;
    updateBar();

    // convert seconds remaining into mm : ss format, and update the label in the ui
    QString strTime = QDateTime::fromTime_t(secsRemaining).toUTC().toString("mm : ss");
    ui->timer->setText(strTime);

    // only start a new session if there's enough battery to do so
    if (remainingBattery > totalTime) {
        // set play button to a 'session' state, so it will resume sessions that are paused
        deviceState = 's';

        // turn on the blue light indicating contact
        ui->blueLight->setStyleSheet("background-color: blue;");

        // if there is an EEG disconnect, wait until that's resolved to start the timer
        // the eeg sites code will automatically start the timer and stuff
        if (eegDisconnect){
            qInfo() << "Waiting for contact to be fully established...";
            ui->description->setText("Waiting for contact to be fully established...");

            disconnectRedLightTimer->start(120);
            beepTimer->start(1000);

            // if 5 minutes passes and no contact is established, turn off the device
            // For testing purposes, this is just five seconds.
            fiveminuteTimer->start(5000);

            return;
        }

        // start the timer to timeout() every second
        sessionTimer->start(1000);
    }
    else {
        deviceState = 'f';
        qInfo() << "There is not enough battery for a session. Please power off and replace the battery.";
        ui->description->setText("There is not enough battery for a session. Please power off and replace the battery.");
    }
}

void MainWindow::sessionLog() {
    ui->sessionLog->setVisible(true);
    ui->sessionLog->clear();

    //name of file we want to open
    QString filename =  "savedSessions.txt";

    QFile file(filename);

    //open the file to read only
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qInfo() << "Unable to open file";
    }
    else{
        QTextStream in(&file);

    //this goes through the file, reads line by line and displays the date to the gui
        while(!in.atEnd()){

            QString session = in.readLine();

            QString date = session.section(" ",0,1);



            QListWidgetItem *add = new QListWidgetItem(date);
            ui->sessionLog->addItem(add);


        }

        file.close();

        // the state of the buttons is the same as a finished session, because the pause, play, and stop buttons do not work,
        // but the menu and power buttons still work
        deviceState = 'f';
    }
}

void MainWindow::timeAndDate() {
    ui->dateTimeEdit->setVisible(true);

    // the state of the buttons is the same as a finished session, because the pause, play, and stop buttons do not work,
    // but the menu and power buttons still work
    deviceState = 'f';
}

void MainWindow::on_dateTimeEdit_dateTimeChanged(const QDateTime &newDateTime)
{
    // automatically update date on change
    dateTime = newDateTime;
}

void MainWindow::countdown()
{
    // this will run at the very start of the session, when 0 seconds have passed
    if (secsRemaining == totalTime) {
        qInfo() << "Now calculating initial dominant average frequency";
        ui->description->setText("Now calculating initial dominant average frequency");
        for (int i = 0; i < 21; i++) {
            int measured = electrodes[i]->measure(testMax, testMin);
            qInfo() << "Electrode" << i + 1 << "Measured:" << measured << "Hz";
            initialFreq.push_back(measured);
            startingOverallBaseline+=measured;
        }
    }

    secsRemaining--;

    // stop the timer when time's up
    if (secsRemaining < 0) {
        sessionTimer->stop();
        saveSession();
        qInfo() << "The session has ended.";
        ui->description->setText("The session has ended. Please press the menu button.");
        deviceState = 'f';

        // turn off the blue light indicating contact
        ui->blueLight->setStyleSheet("background-color: gray;");
    }
    else {
        // convert seconds remaining into mm : ss format, and update the label in the ui
        QString strTime = QDateTime::fromTime_t(secsRemaining).toUTC().toString("mm : ss");
        ui->timer->setText(strTime);
    }

    // the amount of time that has passed represents which round the session is at
    int timeElapsed = totalTime - secsRemaining;

    if (timeElapsed == calculateSecs) {
        lightTimer->start(120);
        qInfo() << Qt::endl << "Round 1 in progress. Adding 5 Hz";
        ui->description->setText("Round 1 in progress. Adding 5 Hz");

        // add 5 Hz to frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "- Administering treatment:" << electrodes[i]->administerTreatment(5) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs) {
        lightTimer->stop();
        turnOffGreenLight();
        qInfo() << Qt::endl << "Now recalculating dominant average frequency.";
        ui->description->setText("Now recalculating dominant average frequency.");

        // recalculate dominant average frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "Measured:" << electrodes[i]->measure(testMax, testMin) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs + calculateSecs) {
        lightTimer->start(120);
        qInfo() << Qt::endl << "Round 2 in progress. Adding 10 Hz";
        ui->description->setText("Round 2 in progress. Adding 10 Hz");

        // add 10 Hz to frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "- Administering treatment:" << electrodes[i]->administerTreatment(10) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs + calculateSecs + treatmentSecs) {
        lightTimer->stop();
        turnOffGreenLight();
        qInfo() << Qt::endl << "Now recalculating dominant average frequency.";
        ui->description->setText("Now recalculating dominant average frequency.");

        // recalculate dominant average frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "Measured:" << electrodes[i]->measure(testMax, testMin) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs + calculateSecs + treatmentSecs + calculateSecs) {
        lightTimer->start(120);
        qInfo() << Qt::endl << "Round 3 in progress. Adding 15 Hz";
        ui->description->setText("Round 3 in progress. Adding 15 Hz");

        // add 15 Hz to frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "- Administering treatment:" << electrodes[i]->administerTreatment(15) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs + calculateSecs + treatmentSecs + calculateSecs + treatmentSecs) {
        lightTimer->stop();
        turnOffGreenLight();
        qInfo() << Qt::endl << "Now recalculating dominant average frequency.";
        ui->description->setText("Now recalculating dominant average frequency.");

        // recalculate dominant average frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "Measured:" << electrodes[i]->measure(testMax, testMin) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs + calculateSecs + treatmentSecs + calculateSecs + treatmentSecs + calculateSecs) {
        lightTimer->start(120);
        qInfo() << Qt::endl << "Round 4 in progress. Adding 20 Hz";
        ui->description->setText("Round 4 in progress. Adding 20 Hz");

        // add 20 Hz to frequency
        for (int i = 0; i < 21; i++) {
            qInfo() << "Electrode" << i + 1 << "- Administering treatment:" << electrodes[i]->administerTreatment(20) << "Hz";
        }
    }
    else if (timeElapsed == calculateSecs + treatmentSecs + calculateSecs + treatmentSecs + calculateSecs + treatmentSecs + calculateSecs + treatmentSecs) {
        lightTimer->stop();
        turnOffGreenLight();
        qInfo() << Qt::endl << "Now calculating final dominant average frequency.";
        ui->description->setText("Now calculating final dominant average frequency.");

        // calculate final dominant average frequency
        for (int i = 0; i < 21; i++) {
            int measured = electrodes[i]->measure(testMax, testMin);
            qInfo() << "Electrode" << i + 1 << "Measured:" << measured << "Hz";
            finalFreq.push_back(measured);
            endOverallBaseline+=measured;
        }
    }
}


void MainWindow::saveSession(){

    // Get the selected date and time from the QDateTimeEdit widget
    QDateTime currentDate = ui->dateTimeEdit->dateTime();

    // Convert the QDateTime object to a string
    QString currentDateString = currentDate.toString("yyyy-MM-dd hh:mm:ss");

    //name of file to open
    QString filename = "savedSessions.txt";

    //this opens the file , and will create the file if does not exist
    QFile file(filename);
    if(!file.open(QIODevice::Append | QIODevice::Text)){
       qInfo() << "failed to open file";
    }
    else{

        QTextStream session(&file);

        //adds the string to the file followed by a new line
        session << currentDateString << " " <<  startingOverallBaseline/21 << " " << endOverallBaseline/21 << " ";

        startingOverallBaseline =0;

        endOverallBaseline =0;

        while(!initialFreq.empty()){
            session << initialFreq.front() << " ";
            initialFreq.erase(initialFreq.begin());
        }

        while(!finalFreq.empty()){
            session << finalFreq.front() << " ";
            finalFreq.erase(finalFreq.begin());
        }


        qInfo() << initialFreq;
        qInfo() << finalFreq;

        qInfo() << startingOverallBaseline;
        qInfo() <<endOverallBaseline;

        session << "\n";

        file.close();
    }
}


void MainWindow::depleteBattery()
{
    remainingBattery--;

    // stop the timer when time's up, and turn off the device
    if (remainingBattery < 0) {
        batteryTimer->stop();

        // turn off the device
        turnDeviceOff();
    }
    else {
        int batteryPercentage = remainingBattery * 100 / totalBattery;

        // turn battery red at 10 percent
        if (batteryPercentage <= 10) {
            ui->battery->setStyleSheet("QProgressBar::chunk {border: 2px solid black; background-color: red;}");
        }

        // update battery representation
        ui->battery->setValue(batteryPercentage);
    }
}

void MainWindow::resetBattery(){
    remainingBattery = totalBattery;
    ui->battery->setValue(100);
}

void MainWindow::flashGreenLight() {
    if (greenLightOn) {
        turnOffGreenLight();
    }
    else {
        ui->greenLight->setStyleSheet("background-color: green;");
        greenLightOn = true;
    }
}

void MainWindow::flashRedLight() {
    if (redLightOn){
        turnOffRedLight();
    }
    else {
        ui->redLight->setStyleSheet("background-color: red;");
        redLightOn = true;
    }
}

void MainWindow::turnOffGreenLight() {
    ui->greenLight->setStyleSheet("background-color: gray;");
    greenLightOn = false;
}

void MainWindow::turnOffRedLight(){
    ui->redLight->setStyleSheet("background-color: gray;");
    redLightOn = false;
}

void MainWindow::beep() {
    qInfo("The device beeps.");
}

void MainWindow::updateBar()
{
    // update the progress bar with percentage of time remaining
    ui->progressBar->setValue((totalTime - secsRemaining) * 100 / totalTime);
}

void MainWindow::checkForSensorDisconnect(){
    // grab a reference to every checkbox in the grid layout
    QList<QCheckBox*> sensors = ui->groupBoxSensors->findChildren<QCheckBox*>();

    // assume all checkboxes are checked
    bool is_all_connected = true;

    // loop through every checkbox
    foreach(QCheckBox* sensor, sensors){
        if (!(sensor->isChecked())){
            is_all_connected = false;
        }
    }

    // if there is an EEG disconnect (not all checkboxes are checked)
    if (!is_all_connected){
        ui->labelSensors->setText("Disconnect");

        eegDisconnect = true;

        // this code only runs if a session is currently happening
        if (deviceState == 's') {
            sessionTimer->stop();

            ui->description->setText("Contact lost. Please establish contact with all EEG sites...");

            disconnectRedLightTimer->start(120);
            beepTimer->start(1000);

            // if 5 minutes passes and no contact is established, turn off the device
            // This is five seconds for testing purposes.
            fiveminuteTimer->start(5000);
        }

    }
    else {
        ui->labelSensors->setText("All good");

        eegDisconnect = false;

        if (deviceState == 's'){
            sessionTimer->start(1000);

            ui->description->setText("Contact re-established.");

            disconnectRedLightTimer->stop();
            turnOffRedLight();
            beepTimer->stop();

            // stop the 5 minute timer
            fiveminuteTimer->stop();
        }
    }
}

void MainWindow::setAllSensors(bool state){
    // grab a reference to every checkbox in the grid layout
    QList<QCheckBox*> sensors = ui->groupBoxSensors->findChildren<QCheckBox*>();

    // loop through every checkbox
    foreach(QCheckBox* sensor, sensors){
        sensor->setChecked(state);
    }
}

// the next 21 functions are identical and just run checkForSensorDisconnect
void MainWindow::on_checkBoxSensor_1_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_2_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_3_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_4_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_5_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_6_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_7_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_8_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_9_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_10_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_11_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_12_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_13_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_14_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_15_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_16_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_17_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_18_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_19_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_20_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_checkBoxSensor_21_stateChanged(int arg1)
{
    checkForSensorDisconnect();
}


void MainWindow::on_pushButtonSensorsOn_clicked()
{
    setAllSensors(true);
}


void MainWindow::on_pushButtonSensorsOff_clicked()
{
    setAllSensors(false);
}


void MainWindow::on_replaceBatteryButton_clicked()
{
    resetBattery();
}


void MainWindow::on_testGamma_clicked()
{
    testMax = 100;
    testMin = 30;
    cout << "Next usage will demonstrate Gamma waves" << endl;
}


void MainWindow::on_testBeta_clicked()
{
    testMax = 30;
    testMin = 12;
    cout << "Next usage will demonstrate Beta waves" << endl;
}


void MainWindow::on_testAlpha_clicked()
{
    testMax = 12;
    testMin = 8;
    cout << "Next usage will demonstrate Alpha waves" << endl;
}


void MainWindow::on_testTheta_clicked()
{
    testMax = 8;
    testMin = 4;
    cout << "Next usage will demonstrate Theta waves" << endl;
}


void MainWindow::on_testDelta_clicked()
{
    testMax = 4;
    testMin = 1;
    cout << "Next usage will demonstrate Delta waves" << endl;
}

