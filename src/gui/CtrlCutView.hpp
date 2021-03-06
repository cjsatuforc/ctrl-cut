/*
 * Ctrl-Cut - Laser cutter toolchain
 * See LICENSE file
 * Copyright (C) 2011 Amir Hassan <amir@viel-zu.org> and Marius kintel <kintel@kintel.net>
 */
#ifndef CTRLCUTVIEW_H_
#define CTRLCUTVIEW_H_
 
#include "NavigationView.hpp"

class CtrlCutView : public NavigationView
{
  Q_OBJECT
public:
  CtrlCutView(QWidget* parent = NULL);

signals:
  void fileDropped(const QString &filename);

private:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);
};
 
#endif
