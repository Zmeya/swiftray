#include <undo.h>
#include <canvas/canvas.h>

JoinedEventPtr operator+(const EventPtr &a, const EventPtr &b) {
  return JoinedEventPtr(new JoinedEvent({a, b}));
}

JoinedEventPtr &operator<<(JoinedEventPtr &a, const EventPtr &b) {
  a->events << b;
  return a;
}

JoinedEventPtr &operator<<(JoinedEventPtr &a, BaseUndoEvent *b) {
  a->events << EventPtr(b);
  return a;
}

void AddLayerEvent::undo() {
  Canvas::document().removeLayer(layer_);
}

void AddLayerEvent::redo() {
  Canvas::document().addLayer(layer_);
}

void RemoveLayerEvent::undo() {
  Canvas::document().addLayer(layer_);
}

void RemoveLayerEvent::redo() {
  Canvas::document().removeLayer(layer_);
}

void AddShapeEvent::undo() {
  qInfo() << "Undo add shape" << (int) shape_->type();
  layer_->removeShape(shape_);
}

void AddShapeEvent::redo() {
  qInfo() << "Redo add shape" << (int) shape_->type();
  layer_->addShape(shape_);
}


void RemoveShapeEvent::undo() {
  qInfo() << "Undo remove shape" << (int) shape_->type();
  layer_->addShape(shape_);
}

void RemoveShapeEvent::redo() {
  qInfo() << "Undo add shape" << (int) shape_->type();
  layer_->removeShape(shape_);
}

SelectionEvent::SelectionEvent() {
  origin_selections_.clear();
  origin_selections_.append(Canvas::document().selections());
}

void SelectionEvent::undo() {
  qInfo() << "Undo selection event" << origin_selections_.size();
  redo_selections_.clear();
  redo_selections_.append(Canvas::document().selections());
  Canvas::document().setSelections(origin_selections_);
}

void SelectionEvent::redo() {
  qInfo() << "Redo selection event" << redo_selections_.size();
  Canvas::document().setSelections(redo_selections_);
}

