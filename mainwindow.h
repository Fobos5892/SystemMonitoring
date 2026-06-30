#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ViewModels/sensormodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    SensorModel* getModel() const { return m_model; }

private:
    Ui::MainWindow *ui;
    SensorModel *m_model = nullptr;
};
#endif // MAINWINDOW_H
