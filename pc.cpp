#include "pc.h"
#include "ui_pc.h"

pc::pc(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pc)
{
    ui->setupUi(this);
    freq = 1;
    ui->graph->xAxis->setLabel("Seconds");
    ui->graph->yAxis->setLabel("Voltage (micro volts)");
    ui->graph->xAxis->setRange(0, 1);
    ui->graph->yAxis->setRange(0, 200);
    ui->graph->setWindowTitle("Before treatment baseline EEG graph");
    ui->graph->replot();


    //populate menu
    populateMenu();
    ui->menuButton->setVisible(false);
    ui->startingBaseline->setVisible(false);
    ui->endBaseline->setVisible(false);
    ui->dataTable->setVisible(false);
    ui->graph->setVisible(false);


    //connect buttons
    connect(ui->upButton,SIGNAL(released()),this, SLOT(upButton()));
    connect(ui->downButton,SIGNAL(released()),this, SLOT(downButton()));
    connect(ui->selectButton,SIGNAL(released()),this, SLOT(selectButton()));
    connect(ui->menuButton,SIGNAL(released()),this, SLOT(menuButton()));
    connect(ui->refreshButton,SIGNAL(released()),this, SLOT(refreshButton()));

}

pc::~pc()
{
    delete ui;
}

//buttons
void pc::upButton(){
    // navigate the menu in the up direction, if possible
    if (ui->menu->currentRow() > 0) {
        ui->menu->setCurrentRow(ui->menu->currentRow()-1);
    }
    else{
        ui->menu->setCurrentRow(numSessions-1);
    }
}

void pc::downButton(){
    // navigate the menu in the down direction, if possible





        ui->menu->setCurrentRow(ui->menu->currentRow()+1);

        if(ui->menu->currentRow()==-1){
           ui->menu->setCurrentRow(0);
        }

}


void pc::selectButton(){
    showData(ui->menu->currentRow());
}


void pc::menuButton(){
   showMenu();
   ui->graph->clearGraphs();
   ui->graph->replot();
}

//populating data from files
void pc::populateMenu(){


    ui->menu->clear();
    numSessions=0;
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

            ui->menu->addItem(add);
            numSessions++;


        }



        file.close();
}

}

void pc::showData(int sessionNum){
    //changing appearance
    ui->menu->setVisible(false);
    ui->downButton->setVisible(false);
    ui->upButton->setVisible(false);
    ui->selectButton->setVisible(false);
    ui->refreshButton->setVisible(false);


    ui->menuButton->setVisible(true);
    ui->startingBaseline->setVisible(true);
    ui->endBaseline->setVisible(true);
    ui->dataTable->setVisible(true);
    ui->graph->setVisible(true);



    //get data from file for session number sessionNum
    extractData(sessionNum);

    QString startText =  "Starting Baseline: " + QString::number(startBaseline);
    QString endText =  "Final Baseline: " + QString::number(endBaseline);


    ui->startingBaseline->setText(startText);
    ui->endBaseline->setText(endText);

    populateTable();



}

void pc::showMenu(){
    ui->menu->setVisible(true);
    ui->downButton->setVisible(true);
    ui->upButton->setVisible(true);
    ui->selectButton->setVisible(true);
    ui->refreshButton->setVisible(true);
    ui->menuButton->setVisible(false);
    ui->startingBaseline->setVisible(false);
    ui->endBaseline->setVisible(false);
    ui->dataTable->setVisible(false);
    ui->graph->setVisible(false);
}

void pc::refreshButton(){
    populateMenu();
}


void pc::extractData(int lineNumber){

    startBaseline =0;
    endBaseline=0;
    initFreq.clear();
    endFreq.clear();


    QString filename =  "savedSessions.txt";

    QFile file(filename);

    //open the file to read only
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qInfo() << "Unable to open file";
    }
    else{
        QTextStream in(&file);

        int currentLine=0;
        QString session;
    //this goes through the file, reads line by line and displays the date to the gui
        while (!in.atEnd()) {
               session = in.readLine();

               if(lineNumber==currentLine){
                   break;
               }
               currentLine++;
           }



        file.close();




    QString date = session.section(' ', 0, 1);


    QStringList intValues = session.section(' ', 2).split(' ');






    //these  are the two values you need for the graph
    startBaseline = intValues[0].toInt();
    endBaseline = intValues[1].toInt();

        freq = startBaseline;
        double period = freq * 6.28;

        QVector<double> x(5000), y(5000);
        random_device rd;
        mt19937 generator(rd());

        int offset = 1;
        int interval;
        if (freq <= 4) {
            offset = 5;
            interval = 100;
        } else if (freq <= 8) {
            offset = 10;
            interval = 50;
        } else if (freq <= 12) {
            offset = 25;
            interval = 25;
        } else if (freq <= 30) {
            offset = 40;
            interval = 15;
        } else if (freq <= 50) {
            offset = 60;
            interval = 10;
        } else {
            offset = 80;
            interval = 5;
        }

        uniform_int_distribution<int> distribution(1, offset);
        int rand = 1;

        for (int i = 0; i < 5000; i++) {
            if (i % interval == 0) {
                rand = distribution(generator);
            }
            x[i] = i / 5000.0;
            y[i] = rand * qSin(period * x[i]) + 100;
        }
        ui->graph->addGraph();
        ui->graph->graph(0)->setData(x, y);
        ui->graph->replot();






    for(int x=2; x<23; x++){
        initFreq.push_back(intValues[x].toInt());
    }

    for(int i=23; i<intValues.size()-1; i++){
        endFreq.push_back(intValues[i].toInt());
    }

    //qInfo() << startBaseline << endBaseline << initFreq << endFreq;



}
}


void pc::populateTable(){
    ui->dataTable->clear();


    ui->dataTable->setRowCount(2);
    ui->dataTable->setColumnCount(21);


    QStringList tableHeaders = { "Initial Frequency", "Final Frequency"};

    ui->dataTable->setVerticalHeaderLabels(tableHeaders);


    for(int x=0; x<21; x++){
        QTableWidgetItem  *init = new QTableWidgetItem(QString::number(initFreq[x]));
        QTableWidgetItem  *final = new QTableWidgetItem(QString::number(endFreq[x]));

        ui->dataTable->setItem(0,x,init);
        ui->dataTable->setItem(1,x,final);

    }


}
