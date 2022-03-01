#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickWindow>
#include <QQuickWidget>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QMenu>
#include <QToolButton>
#include <widgets/components/layer-list-item.h>
#include <widgets/panels/transform-panel.h>
#include <widgets/panels/doc-panel.h>
#include <widgets/panels/layer-panel.h>
#include <widgets/panels/font-panel.h>
#include <widgets/panels/image-panel.h>
#include <widgets/panels/jogging-panel.h>
#include <windows/machine-manager.h>
#include <windows/preferences-window.h>
#include <windows/gcode-player.h>
#include <windows/welcome-dialog.h>
#include <canvas/canvas.h>
#include <widgets/base-container.h>

#include <windows/job-dashboard-dialog.h>
#include <connection/serial-job.h>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow, BaseContainer {
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

  ~MainWindow();

  bool event(QEvent *e) override;

  void loadCanvas();

  void loadStyles() override;

  void closeEvent(QCloseEvent *event) override;

  Canvas *canvas() const;

signals:

  void presetSettingsChanged();

  void machineSettingsChanged();

  void toolbarTransformChanged(double x, double y, double r, double w, double h);

public slots:
  void onStartNewJob();
  void onStartNewJobFromDashboard();
  void onStopJob();
  void onPauseJob();
  void onResumeJob();

private slots:

  void canvasLoaded(QQuickWidget::Status status);

  void sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);

  void updateMode();

  void updateSelections();

  void updateScale();

  void updateToolbarTransform();

  void onScaleMinusClicked();

  void onScalePlusClicked();

  void openFile();

  void openImageFile();

  void replaceImage();

  void imageSelected(const QImage image);

  void importImage(QString file_name);

  void setCanvasContextMenu();

  void setConnectionToolBar();

  void setToolbarFont();

  void setToolbarTransform();

  //void setToolbarImage();

  void setScaleBlock();

  void showCanvasPopMenu();

  void showWelcomeDialog();

  void showJoggingPanel();

  void genPreviewWindow();

private:

  void loadWidgets() override;

  void loadSettings() override;

  void registerEvents() override;

  Ui::MainWindow *ui;
  Canvas *canvas_;
  double x_, y_, r_, w_, h_;

  // Context menu of canvas
  QMenu *popMenu_;
  QAction *cutAction_;
  QAction *copyAction_;
  QAction *pasteAction_;
  QAction *pasteInPlaceAction_;
  QAction *duplicateAction_;
  QAction *deleteAction_;
  QAction *groupAction_;
  QAction *ungroupAction_;

  QPushButton *scale_block_;
  QMenu *popScaleMenu_;
  QComboBox* baudComboBox_;
  QComboBox* portComboBox_;

  TransformPanel *transform_panel_;
  GCodePlayer *gcode_player_;
  JobDashboardDialog *job_dashboard_;
  DocPanel *doc_panel_;
  FontPanel *font_panel_;
  ImagePanel *image_panel_;
  LayerPanel *layer_panel_;
  MachineManager *machine_manager_;
  WelcomeDialog *welcome_dialog_;
  JoggingPanel *jogging_panel_;
  PreferencesWindow *preferences_window_;

#ifndef Q_OS_IOS
    QList<SerialJob *> jobs_;
#endif

  void newFile();
  void saveFile();
  void saveAsFile();
  void exportGCodeFile();
  void importGCodeFile();
  void generateGcode();
  void generateJob();
  bool handleUnsavedChange();
};

#endif // MAINWINDOW_H
