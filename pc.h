#ifndef PC_H
#define PC_H

#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <vector>
#include <cmath>
#include <random>
using namespace std;


QT_BEGIN_NAMESPACE
namespace Ui { class pc; }
QT_END_NAMESPACE

class pc : public QMainWindow
{
    Q_OBJECT

public:
    pc(QWidget *parent = nullptr);
    ~pc();

private:
    Ui::pc *ui;

    int startBaseline;
    int endBaseline;
    vector<int> initFreq;
    vector<int> endFreq;


    int numSessions;

    void populateMenu();
    void showData(int sessionNum);
    void showMenu();
    void extractData(int sessionNum);
    void populateTable();
    int freq;

private slots:

    void upButton();
    void downButton();
    void selectButton();
    void menuButton();
    void refreshButton();
};
#endif // PC_H
