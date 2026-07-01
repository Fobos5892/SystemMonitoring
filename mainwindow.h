#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Data/sensorstatistics.h"
#include "ViewModels/sensormodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ThreadOrchestrator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    SensorModel* getModel() const { return m_model; }

private slots:
    void onConnectButtonClicked();
    void onClearDatabaseClicked();
    void onApplyFilterClicked();
    void onFilterFieldChanged(int index);
    void onSensorStatisticsUpdated(const SensorStatistics &stats);

private:
    void normalizeNumericFilterInputs();
    void setGenerationRunning(bool running);
    void setLoadingOverlayVisible(bool visible);
    void setControlsEnabled(bool enabled);
    QString buildFilterCondition() const;

    Ui::MainWindow *ui;
    SensorModel *m_model = nullptr;
    ThreadOrchestrator *m_orchestrator = nullptr;
    bool m_isGenerating = false;
};
#endif // MAINWINDOW_H
