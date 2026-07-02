#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QTimer>
#include "Domain/filterlimits.h"
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
    void onResetFilterClicked();
    void onFilterFieldChanged(int index);
    void onStatisticsLabelsChanged();
    void onConnectionStatusChanged(Telemetry::ConnectionStatus status);

private:
    static constexpr int MIN_TABLE_COLUMN_WIDTH = 60;
    static constexpr int RECORD_ID_COLUMN_WIDTH = 90;
    static constexpr int SENSOR_ID_COLUMN_WIDTH = 90;
    static constexpr int VALUE_COLUMN_WIDTH = 130;
    static constexpr int TIMESTAMP_COLUMN_WIDTH = 260;

    void syncFilterViewModelFromUi();
    void resetFilterControlsToDefault();
    void normalizeNumericFilterInputs();
    void setGenerationRunning(bool running);
    void setLoadingOverlayVisible(bool visible);
    void acquireUiLock();
    void releaseUiLock();
    void applyUiLockState();
    void startLoadingAnimation();
    void stopLoadingAnimation();
    bool isUiInteractive() const;
    void bindStatisticsLabels();
    void configureFilterControls();
    void configureTableColumns();
    void updateFollowMode();
    void updateViewportZone();
    void onScrollActivity();
    void onScrollIdleTimeout();
    void setTableScrollLocked(bool locked);
    int computeFilterRequestLimit() const;

    static constexpr int SCROLL_IDLE_MS = 200;

    QScopedPointer<Ui::MainWindow> ui;
    QScopedPointer<TelemetryFacade> facade;
    QScopedPointer<QTimer> loadingAnimationTimer;
    QScopedPointer<QTimer> scrollIdleTimer;
    bool isGenerating = false;
    int uiLockDepth = 0;
    int loadingAnimationFrame = 0;
    bool tableScrollLocked = false;
};
#endif // MAINWINDOW_H
