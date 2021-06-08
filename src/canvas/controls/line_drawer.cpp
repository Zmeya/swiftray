#include <QPainterPath>
#include <canvas/controls/line_drawer.h>
#include <cmath>
#include <shape/path_shape.h>

bool LineDrawer::mousePressEvent(QMouseEvent *e) {
    CanvasControl::mousePressEvent(e);
    return false;
}

bool LineDrawer::mouseMoveEvent(QMouseEvent *e) {
    if (scene().mode() != Scene::Mode::DRAWING_LINE)
        return false;
    cursor_ = scene().getCanvasCoord(e->pos());
    return true;
}

bool LineDrawer::mouseReleaseEvent(QMouseEvent *e) {
    if (scene().mode() != Scene::Mode::DRAWING_LINE)
        return false;
    QPainterPath path;
    path.moveTo(dragged_from_canvas_);
    path.lineTo(scene().getCanvasCoord(e->pos()));
    ShapePtr new_line = make_shared<PathShape>(path);
    scene().stackStep();
    scene().activeLayer().addShape(new_line);
    scene().setSelection(new_line);
    scene().setMode(Scene::Mode::SELECTING);
    return true;
}

void LineDrawer::paint(QPainter *painter) {
    if (scene().mode() != Scene::Mode::DRAWING_LINE)
        return;
    if (cursor_ == QPointF(0, 0))
        return;
    QPen pen(scene().activeLayer().color(), 3, Qt::SolidLine);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawLine(dragged_from_canvas_, cursor_);
}

void LineDrawer::reset() { cursor_ = QPointF(); }