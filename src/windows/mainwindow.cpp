#include <QDebug>
#include <QFileDialog>
#include <QListWidget>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWidget>
#include <QAbstractItemView>
#include <boost/range/adaptor/reversed.hpp>
#include <shape/bitmap-shape.h>
#include <widgets/components/canvas-text-edit.h>
#include <windows/mainwindow.h>
#include <windows/osxwindow.h>
#include <windows/preview-window.h>
#include <gcode/toolpath-exporter.h>
#include <gcode/generators/gcode-generator.h>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
     QMainWindow(parent),
     ui(new Ui::MainWindow),
     canvas_(nullptr) {
  ui->setupUi(this);
  loadQSS();
  loadSettings();
  loadCanvas();
  loadWidgets();
  registerEvents();
  updateLayers();
  updateMode();
  updateSelections();
}

void MainWindow::loadSettings() {
  QSettings settings;
  restoreGeometry(settings.value("window/geometry").toByteArray());
  restoreState(settings.value("window/windowState").toByteArray());
}

void MainWindow::loadCanvas() {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  // QPaintedItem in Qt6 does not support Metal rendering yet, so it will be slow using metal RHI
  qInfo() << "Falling back to OpenGLRhi in QT6";
  ((QQuickWindow *)ui->quickWidget)->setGraphicsApi(QSGRendererInterface::OpenGLRhi);
  connect(ui->quickWidget, &QQuickWidget::sceneGraphError, this, &MainWindow::sceneGraphError);
#endif
  connect(ui->quickWidget, &QQuickWidget::statusChanged, this, &MainWindow::canvasLoaded);

  QUrl source("qrc:/src/windows/main.qml");
  ui->quickWidget->setResizeMode(QQuickWidget::ResizeMode::SizeRootObjectToView);
  ui->quickWidget->setSource(source);
  ui->quickWidget->show();
}

void MainWindow::loadQSS() {
  QFile file(":/styles/beambird.qss");
  file.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(file.readAll());
  setStyleSheet(styleSheet);
}

void MainWindow::openFile() {
  QString file_name = QFileDialog::getOpenFileName(this, "Open SVG", ".", tr("SVG Files (*.svg)", "BVG Files (*.bvg)"));

  if (!QFile::exists(file_name))
    return;

  QFile file(file_name);

  if (file.open(QFile::ReadOnly)) {
    QByteArray data = file.readAll();
    qInfo() << "File size:" << data.size();
    canvas_->loadSVG(data);
  }
}

void MainWindow::openImageFile() {
  QString file_name = QFileDialog::getOpenFileName(this, "Open Image", ".", tr("Image Files (*.png *.jpg)"));

  if (!QFile::exists(file_name))
    return;

  QImage image;

  if (image.load(file_name)) {
    qInfo() << "File size:" << image.size();
    canvas_->importImage(image);
  }
}

void MainWindow::canvasLoaded(QQuickWidget::Status status) {
  if (status == QQuickWidget::Error) {
    const auto widgetErrors = this->ui->quickWidget->errors();

    for (const QQmlError &error : widgetErrors)
      qInfo() << error.toString();

    Q_ASSERT_X(false, "QQuickWidget Initialization", "QQuickWidget failed to initialize");
  }

  canvas_ = ui->quickWidget->rootObject()->findChildren<Canvas *>().first();
  canvas_->setWidget(ui->quickWidget);
  // TODO (Chanage the owner of text_box_ to mainwindow)
  canvas_->text_input_ = new CanvasTextEdit(this);
  canvas_->text_input_->setGeometry(0, 0, 0, 0);
  canvas_->text_input_->setStyleSheet("border:0");
}

void MainWindow::updateLayers() {
  ui->layerList->clear();

  for (auto &layer : boost::adaptors::reverse(canvas_->document().layers())) {
    bool active = canvas_->document().activeLayer() == layer.get();
    LayerPtr editable_layer = layer;
    auto *list_widget = new LayerListItem(ui->layerList->parentWidget(),
                                          canvas_,
                                          editable_layer,
                                          active);
    auto *list_item = new QListWidgetItem(ui->layerList);
    auto size = list_widget->size();
    list_item->setSizeHint(size);
    ui->layerList->setItemWidget(list_item, list_widget);

    if (active) {
      ui->layerList->setCurrentItem(list_item);
    }
  }

  if (ui->layerList->currentItem()) {
    ui->layerList->scrollToItem(ui->layerList->currentItem(), QAbstractItemView::PositionAtCenter);
  }

  layer_params_panel_->updateLayer(canvas_->document().activeLayer());
}

void MainWindow::layerOrderChanged(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                   const QModelIndex &destinationParent, int destinationRow) {
  QList<LayerPtr> new_order;

  for (int i = ui->layerList->count() - 1; i >= 0; i--) {
    new_order << dynamic_cast<LayerListItem *>(ui->layerList->itemWidget(ui->layerList->item(i)))->layer_;
  }

  canvas_->setLayerOrder(new_order);
}

bool MainWindow::event(QEvent *e) {
  switch (e->type()) {
    case QEvent::CursorChange:
    case QEvent::UpdateRequest:
      break;

    case QEvent::NativeGesture:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      canvas_->event(e);
      return true;

    default:
      // qInfo() << "Event" << e;
      break;
  }

  return QMainWindow::event(e);
}

void MainWindow::sceneGraphError(QQuickWindow::SceneGraphError, const QString &message) {
  // statusBar()->showMessage(message);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::updateMode() {
  // TODO: use action group to do button state exclusive
  ui->actionSelect->setChecked(false);
  ui->actionDrawRect->setChecked(false);
  ui->actionDrawLine->setChecked(false);
  ui->actionDrawOval->setChecked(false);
  ui->actionDrawPath->setChecked(false);
  ui->actionDrawText->setChecked(false);
  ui->actionDrawPolygon->setChecked(false);

  switch (canvas_->mode()) {
    case Canvas::Mode::Selecting:
    case Canvas::Mode::MultiSelecting:
      ui->actionSelect->setChecked(true);
      break;

    case Canvas::Mode::LineDrawing:
      ui->actionDrawLine->setChecked(true);
      break;

    case Canvas::Mode::RectDrawing:
      ui->actionDrawRect->setChecked(true);
      break;

    case Canvas::Mode::OvalDrawing:
      ui->actionDrawOval->setChecked(true);
      break;

    case Canvas::Mode::PathDrawing:
      ui->actionDrawPath->setChecked(true);
      break;

    case Canvas::Mode::TextDrawing:
      ui->actionDrawText->setChecked(true);
      break;

    default:
      break;
  }
}

void MainWindow::updateSelections() {
  QList<ShapePtr> &items = canvas_->document().selections();
  bool all_group = items.size() > 0;
  bool all_path = items.size() > 0;
  for (auto &shape : canvas_->document().selections()) {
    if (shape->type() != Shape::Type::Group) all_group = false;
    if (shape->type() != Shape::Type::Path && shape->type() != Shape::Type::Text) all_path = false;
  }
  ui->actionGroupBtn->setEnabled(items.size() > 1);
  ui->actionUngroupBtn->setEnabled(all_group);
  ui->actionUnionBtn->setEnabled(all_path); // Union can be done with the shape itself if it contains sub polygons
  ui->actionSubtractBtn->setEnabled(items.size() == 2 && all_path);
  ui->actionDiffBtn->setEnabled(items.size() == 2 && all_path);
  ui->actionIntersectBtn->setEnabled(items.size() == 2 && all_path);
  ui->actionHFlip->setEnabled(!items.empty());
  ui->actionVFlip->setEnabled(!items.empty());

  ui->actionAlignTop->setEnabled(items.size() > 1);
  ui->actionAlignVCenter->setEnabled(items.size() > 1);
  ui->actionAlignBottom->setEnabled(items.size() > 1);
  ui->actionAlignLeft->setEnabled(items.size() > 1);
  ui->actionAlignHCenter->setEnabled(items.size() > 1);
  ui->actionAlignRight->setEnabled(items.size() > 1);

  setOSXWindowTitleColor(this);
}

void MainWindow::loadWidgets() {
  assert(canvas_ != nullptr);
  // Add custom panels
  transform_panel_ = new TransformPanel(ui->objectParamDock, canvas_);
  layer_params_panel_ = new LayerParamsPanel(ui->layerDockContents, canvas_);
  gcode_player_ = new GCodePlayer(ui->serialPortDock);
  font_panel_ = new FontPanel(ui->fontDock, canvas_);
  doc_panel_ = new DocPanel(ui->documentDock, canvas_);
  machine_manager_ = new MachineManager(this);
  ui->objectParamDock->setWidget(transform_panel_);
  ui->serialPortDock->setWidget(gcode_player_);
  ui->fontDock->setWidget(font_panel_);
  ui->layerDockContents->layout()->addWidget(layer_params_panel_);
  ui->documentDock->setWidget(doc_panel_);

  // Add floating buttons
  add_layer_btn_ = new QToolButton(ui->layerList);
  add_layer_btn_->setIcon(QIcon(":/images/icon-plus-01.png"));
  add_layer_btn_->setGeometry(QRect(215, 180, 35, 35));
  add_layer_btn_->setIconSize(QSize(24, 24));
  add_layer_btn_->raise();
  add_layer_btn_->show();
}

void MainWindow::registerEvents() {
  // Monitor canvas events
  connect(canvas_, &Canvas::layerChanged, this, &MainWindow::updateLayers);
  connect(canvas_, &Canvas::modeChanged, this, &MainWindow::updateMode);
  connect(canvas_, &Canvas::selectionsChanged, this, &MainWindow::updateSelections);

  // Monitor UI events
  connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
  connect(ui->actionClose, &QAction::triggered, this, &MainWindow::close);
  connect(ui->actionCut, &QAction::triggered, canvas_, &Canvas::editCut);
  connect(ui->actionCopy, &QAction::triggered, canvas_, &Canvas::editCopy);
  connect(ui->actionPaste, &QAction::triggered, canvas_, &Canvas::editPaste);
  connect(ui->actionUndo, &QAction::triggered, canvas_, &Canvas::editUndo);
  connect(ui->actionRedo, &QAction::triggered, canvas_, &Canvas::editRedo);
  connect(ui->actionSelect_All, &QAction::triggered, canvas_, &Canvas::editSelectAll);
  connect(ui->actionGroup, &QAction::triggered, canvas_, &Canvas::editGroup);
  connect(ui->actionUngroup, &QAction::triggered, canvas_, &Canvas::editUngroup);
  connect(ui->actionSelect, &QAction::triggered, canvas_, &Canvas::backToSelectMode);
  connect(ui->actionDrawRect, &QAction::triggered, canvas_, &Canvas::editDrawRect);
  connect(ui->actionDrawOval, &QAction::triggered, canvas_, &Canvas::editDrawOval);
  connect(ui->actionDrawLine, &QAction::triggered, canvas_, &Canvas::editDrawLine);
  connect(ui->actionDrawPath, &QAction::triggered, canvas_, &Canvas::editDrawPath);
  connect(ui->actionDrawText, &QAction::triggered, canvas_, &Canvas::editDrawText);
  connect(ui->actionDrawPhoto, &QAction::triggered, this, &MainWindow::openImageFile);
  connect(ui->actionUnionBtn, &QAction::triggered, canvas_, &Canvas::editUnion);
  connect(ui->actionSubtractBtn, &QAction::triggered, canvas_, &Canvas::editSubtract);
  connect(ui->actionIntersectBtn, &QAction::triggered, canvas_, &Canvas::editIntersect);
  connect(ui->actionDiffBtn, &QAction::triggered, canvas_, &Canvas::editDifference);
  connect(ui->actionGroupBtn, &QAction::triggered, canvas_, &Canvas::editGroup);
  connect(ui->actionUngroupBtn, &QAction::triggered, canvas_, &Canvas::editUngroup);
  connect(ui->actionHFlip, &QAction::triggered, canvas_, &Canvas::editHFlip);
  connect(ui->actionVFlip, &QAction::triggered, canvas_, &Canvas::editVFlip);
  connect(ui->actionAlignTop, &QAction::triggered, canvas_, &Canvas::editVAlignTop);
  // TODO (Use one naming convention);
  connect(ui->actionAlignVCenter, &QAction::triggered, canvas_, &Canvas::editVAlignMid);
  connect(ui->actionAlignBottom, &QAction::triggered, canvas_, &Canvas::editVAlignBottom);
  connect(ui->actionAlignLeft, &QAction::triggered, canvas_, &Canvas::editHAlignLeft);
  connect(ui->actionAlignHCenter, &QAction::triggered, canvas_, &Canvas::editHAlignCenter);
  connect(ui->actionAlignRight, &QAction::triggered, canvas_, &Canvas::editHAlignRight);

  connect(ui->layerList->model(), &QAbstractItemModel::rowsMoved, this, &MainWindow::layerOrderChanged);

  // Monitor custom widgets
  connect(add_layer_btn_, &QAbstractButton::clicked, canvas_, &Canvas::addEmptyLayer);

  // Complex callbacks
  connect(ui->actionMachineSettings, &QAction::triggered, [=]() {
    machine_manager_->show();
  });
  connect(ui->actionExportGcode, &QAction::triggered, [=]() {
    auto gen = canvas_->exportGcode();
    PreviewWindow *pw = new PreviewWindow(this);
    pw->setPreviewPath(gen);
    pw->show();

    auto gen_gcode = make_shared<GCodeGenerator>();
    ToolpathExporter exporter(gen_gcode.get());
    exporter.convertStack(canvas_->document().layers());
    gcode_player_->setGCode(QString::fromStdString(gen_gcode->toString()));
  });
  connect(ui->layerList, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
    canvas_->setActiveLayer(dynamic_cast<LayerListItem *>(ui->layerList->itemWidget(item))->layer_);
  });
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QSettings settings;
  settings.setValue("window/geometry", saveGeometry());
  settings.setValue("window/windowState", saveState());
  QMainWindow::closeEvent(event);
}