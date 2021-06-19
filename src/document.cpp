#include <QDebug>
#include <boost/range/adaptor/reversed.hpp>
#include <document.h>
#include <shape/group-shape.h>

Document::Document() noexcept:
     mode_(Mode::Selecting),
     new_layer_id_(1),
     scroll_x_(0),
     scroll_y_(0),
     scale_(1),
     is_recording_undo_(true) {
  width_ = 3000;
  height_ = 2000;
  font_ = QFont("Tahoma", 200, QFont::Bold);
  addLayer();
  connect(this, &Document::selectionsChanged, [=]() {
    for (auto &layer : layers_) {
      layer->flushCache();
    }
  });
}

void Document::setSelection(nullptr_t) {
  setSelections({});
}

void Document::setSelection(ShapePtr &shape) {
  setSelections({shape});
}

void Document::setSelections(const QList<ShapePtr> &new_selections) {
  for (auto &shape : selections_) { shape->setSelected(false); }
  selections_.clear();
  selections_.append(new_selections);
  for (auto &shape : selections_) { shape->setSelected(true); }
  emit selectionsChanged();
}

QList<ShapePtr> &Document::selections() { return selections_; }

void Document::dumpStack(QList<LayerPtr> &stack) {
  qInfo() << "<Stack>";
  for (auto &layer : stack) {
    qInfo() << "  <Layer " << layer.get() << ">";
    for (auto &shape : layer->children()) {
      qInfo() << "    <Shape " << shape.get()
              << " selected =" << shape->selected() << " />";
    }
    qInfo() << "  </Layer>";
  }
  qInfo() << "</Stack>";
}

void Document::undo() {
  if (undo2_stack_.isEmpty()) return;
  CmdPtr evt = undo2_stack_.last();
  evt->undo();
  undo2_stack_.pop_back();
  redo2_stack_ << evt;

  QString active_layer_name = activeLayer()->name();

  // TODO (Fix mode change event and selection, and add layer)
  setActiveLayer(active_layer_name);
  emit selectionsChanged();
  emit layerChanged();
}

void Document::redo() {
  if (redo2_stack_.isEmpty()) return;
  qInfo() << "Stack" << redo2_stack_.size();
  CmdPtr evt = redo2_stack_.last();
  evt->redo();
  redo2_stack_.pop_back();
  undo2_stack_ << evt;

  QString active_layer_name = activeLayer()->name();

  setActiveLayer(active_layer_name);
  emit selectionsChanged();
  emit layerChanged();
}

void Document::execute(Commands::BaseCmd *event) {
  execute(CmdPtr(event));
}

void Document::execute(const CmdPtr &e) {
  redo2_stack_.clear();
  e->redo();
  undo2_stack_.push_back(e);
}


void Document::addLayer() {
  qDebug() << "Add layer";
  layers() << make_shared<Layer>(new_layer_id_++);
  active_layer_ = layers().last();
  emit layerChanged();
}

void Document::addLayer(LayerPtr &layer) {
  layers() << layer->clone();
  active_layer_ = layers().last();
}

void Document::removeLayer(LayerPtr &layer) {
  if (!layers().removeOne(layer)) {
    qInfo() << "Failed to remove layer";
  }
}

void Document::emitAllChanges() {
  emit selectionsChanged();
  emit layerChanged();
  emit modeChanged();
}

Document::Mode Document::mode() const { return mode_; }

void Document::setMode(Mode mode) {
  mode_ = mode;
  emit modeChanged();
}

QPointF Document::getCanvasCoord(QPointF window_coord) const {
  return (window_coord - scroll()) / scale();
}

QPointF Document::scroll() const { return QPointF(scroll_x_, scroll_y_); }

qreal Document::scale() const { return scale_; }

qreal Document::width() const { return width_; }

qreal Document::height() const { return height_; }

void Document::setWidth(qreal width) { width_ = width; }

void Document::setHeight(qreal height) { height_ = height; }

void Document::setScroll(QPointF scroll) {
  scroll_x_ = scroll.x();
  scroll_y_ = scroll.y();
  volatility_timer.restart();
}

QRectF Document::screenRect(QSize screen_size) const {
  return QRectF(getCanvasCoord(QPoint(0, 0)), getCanvasCoord(QPoint(screen_size.width(), screen_size.height())));
}

void Document::setScale(qreal scale) {
  scale_ = scale;
  volatility_timer.restart();
}

void Document::setRecordingUndo(bool recording_undo) { is_recording_undo_ = recording_undo; }

LayerPtr &Document::activeLayer() {
  Q_ASSERT_X(layers_.size() != 0, "Active Layer",
             "Access to active layer when there is no layer");
  Q_ASSERT_X(active_layer_ != nullptr, "Active Layer",
             "Access to active layer is cleaned");
  return active_layer_;
}

bool Document::setActiveLayer(const QString &name) {
  for (auto &layer : layers()) {
    if (layer->name() == name) {
      active_layer_ = layer;
      emit layerChanged();
      return true;
    }
  }

  active_layer_ = layers().last();
  return false;
}

void Document::setActiveLayer(LayerPtr &target_layer) {
  if (!layers_.contains(target_layer)) {
    qInfo() << "Total layers" << layers_.size();
    for (auto &layer : layers_) {
      qInfo() << layer.get();
    }
    qInfo() << "Cannot set active layer" << target_layer.get();
    Q_ASSERT_X(false, "Active Layer",
               "Invalid layer ptr when setting active layer");
  }
  active_layer_ = target_layer;
  emit layerChanged();
}

QList<LayerPtr> &Document::layers() { return layers_; }

void Document::reorderLayers(QList<LayerPtr> &new_order) {
  // TODO (Add undo event)
  layers_ = new_order;
}

void Document::removeSelections() {
  // Remove shapes from its layer
  for (auto &shape : selections_) { shape->layer().removeShape(shape); }
  setSelection(nullptr);
}

ShapePtr Document::hitTest(QPointF canvas_coord) {
  for (auto &layer : layers()) {
    for (auto &shape : boost::adaptors::reverse(layer->children())) {
      if (shape->hitTest(canvas_coord, 5 / scale())) {
        return shape;
      }
    }
  }
  return nullptr;
}

QPointF Document::mousePressedCanvasCoord() const {
  return getCanvasCoord(mouse_pressed_screen_coord_);
}

QPointF Document::mousePressedScreenCoord() const {
  return mouse_pressed_screen_coord_;
}

void Document::setMousePressedScreenCoord(QPointF screen_coord) {
  mouse_pressed_screen_coord_ = screen_coord;
}

void Document::setFont(QFont &font) {
  font_ = font;
}

bool Document::isVolatile() {
  if (volatility_timer.elapsed() < 1000) { return true; }
  return mode_ == Mode::Moving || mode_ == Mode::Rotating || mode_ == Mode::Transforming;
}

const QFont &Document::font() const { return font_; }

void Document::groupSelections() {
  if (selections().empty()) return;

  ShapePtr group_ptr = make_shared<GroupShape>(selections());
  execute(
       Commands::AddShape::shared(activeLayer(), group_ptr) +
       Commands::JoinedCmd::removeSelections() +
       Commands::Select::shared({group_ptr})
  );
}

void Document::ungroupSelections() {
  if (selections().empty()) return;
  ShapePtr group_ptr = selections().first();
  if (group_ptr->type() != Shape::Type::Group) return;

  auto *group = (GroupShape *) group_ptr.get();

  auto cmd = make_shared<Commands::JoinedCmd>();
  for (auto &shape : group->children()) {
    cmd << new Commands::SetTransform(shape.get(), shape->transform() * group->transform());
    cmd << new Commands::SetRotation(shape.get(), shape->rotation() + group->rotation());
    cmd << new Commands::AddShape(activeLayer(), shape);
  }

  cmd << new Commands::Select(group->children());
  cmd << new Commands::RemoveShape(&group_ptr->layer(), group_ptr);

  execute(cmd);
}