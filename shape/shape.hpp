#include <QPainter>
#include <QPainterPath>

#ifndef SHAPE_H
#define SHAPE_H
// Use like struct first (can run faster..)
class Shape {
    public:
        Shape() noexcept;
        QPainterPath path;
        bool selected;
        void simplify();
        QPointF pos() const;
        QRectF boundingRect() const;
        bool testHit(QPointF global_coord, qreal tolerance) const;
        bool testHit(QRectF global_coord_rect) const;
        void cacheSelectionTestingData() ;
        qreal x() const;
        qreal y() const;
        qreal rotation() const;
        qreal scaleX() const;
        qreal scaleY() const;
        qreal setX(qreal x);
        qreal setY(qreal y);
        qreal setRotation(qreal r);
        void setPosition(QPointF pos);
        void rotate(qreal deg);
        void scale(qreal scale_x, qreal scale_y);
        void translate(qreal x, qreal y);
        void transform(QTransform transform);
        void pretransform(QTransform transform);
        QTransform globalTransform() const;
    private:
        QTransform transform_;
        QList<QPointF> selection_testing_points;
        QRectF selection_testing_rect;
};
#endif //SHAPE_H