#ifndef LASERDIALOG_H_
#define LASERDIALOG_H_

#include <QDialog>
#include "ui_LaserDialog.h"
#include "Document.hpp"

class LaserDialog : public QDialog, private Ui::LaserDialog
{
  Q_OBJECT;
public:
  LaserDialog(QWidget *parent = NULL);
  ~LaserDialog();
  
  void applyLaserConfig(Document &document);
  void updateLaserConfig(Document &document);

private slots:
  void on_rasterPowerSlider_valueChanged(int);
  void on_rasterSpeedSlider_valueChanged(int);
  void on_vectorPowerSlider_valueChanged(int);
  void on_vectorSpeedSlider_valueChanged(int);
  void on_vectorFreqSlider_valueChanged(int);
  void on_rasterPower_textEdited(const QString &);
  void on_rasterSpeed_textEdited(const QString &);
  void on_vectorPower_textEdited(const QString &);
  void on_vectorSpeed_textEdited(const QString &);
  void on_vectorFreq_textEdited(const QString &);

private:
};

#endif
