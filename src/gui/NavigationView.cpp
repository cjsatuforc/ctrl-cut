/*
 * Ctrl-Cut - Laser cutter toolchain
 * See LICENSE file
 * Copyright (C) 2011 Amir Hassan <amir@viel-zu.org> and Marius kintel <kintel@kintel.net>
 */
#include "NavigationView.hpp"
 
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextStream>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

#include <math.h>
#include <iostream>
 
NavigationView::NavigationView(QWidget* parent) : QGraphicsView(parent)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setAlignment(Qt::AlignCenter);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  setDragMode(QGraphicsView::ScrollHandDrag);
}
 
void NavigationView::wheelEvent(QWheelEvent* event)
{
  scaleView(pow(2.0, event->delta() / 240.0), mapToScene(event->pos()));
}

void NavigationView::scaleView(double scaleFactor, QPointF center)
{
  double factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
  if (factor < 0.01 || factor > 1000) return;

  // Before we scale, the point under the mouse is center
  scale(scaleFactor, scaleFactor);

  // Where is the mouse now?
  QPointF newp = mapToScene(this->viewport()->mapFromGlobal(QCursor::pos()));

  QPointF diff = newp - center;
  translate(diff.x(), diff.y());
}

void NavigationView::keyPressEvent(QKeyEvent *event)
{
  QRectF g = this->mapToScene(0, 0, this->viewport()->geometry().width(), this->viewport()->geometry().height()).boundingRect();

  switch (event->key()) {
  case Qt::Key_Shift:
    setDragMode(QGraphicsView::RubberBandDrag);
    break;
  case Qt::Key_Plus:
    scaleView(1.2, mapToScene(this->mapFromGlobal(QCursor::pos())));
    break;
  case Qt::Key_Minus:
    scaleView(1 / 1.2, mapToScene(this->mapFromGlobal(QCursor::pos())));
    break;
  default:
    QGraphicsView::keyPressEvent(event);
  }
}

void NavigationView::keyReleaseEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Shift:
    setDragMode(QGraphicsView::ScrollHandDrag);
    break;
  default:
    QGraphicsView::keyReleaseEvent(event);
  }
}

void NavigationView::mouseDoubleClickEvent(QMouseEvent *event)
{
  // centerOn(mapToScene(event->pos()));
  // translate(100,100);

  QPointF p = mapToScene(event->pos());
  printf("%f %f\n", p.x(), p.y());
}

void NavigationView::paintEvent(QPaintEvent *event)
{
  QGraphicsView::paintEvent(event);
  emit matrixUpdated();
}

void NavigationView::updateSceneRect(const QRectF &rect)
{
  //setSceneRect(rect.adjusted(-rect.width(), -rect.height(), rect.width(), rect.height()));
}

