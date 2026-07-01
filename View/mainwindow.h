#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Domain/telemetrytypes.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class TelemetryFacade;
class TelemetryTableModel;
class TelemetryViewModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    TelemetryTableModel* getTableModel() const;

private slots:
    void onConnectButtonClicked();
    void onClearDatabaseClicked();
    void onApplyFilterClicked();
    void onFilterFieldChanged(int index);
    void onStatisticsLabelsChanged();

private:
    void syncFilterViewModelFromUi();
    void normalizeNumericFilterInputs();
    void setGenerationRunning(bool running);
    void setLoadingOverlayVisible(bool visible);
    void setControlsEnabled(bool enabled);
    void bindStatisticsLabels();
    void updateFollowMode();

    Ui::MainWindow *ui;
    TelemetryFacade *facade = nullptr;
    bool isGenerating = false;
};
#endif // MAINWINDOW_H
