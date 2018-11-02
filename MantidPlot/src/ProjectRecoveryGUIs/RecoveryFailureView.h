// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef RECOVERYFAILUREVIEW_H
#define RECOVERYFAILUREVIEW_H

#include "ProjectRecoveryPresenter.h"
#include <QDialog>
#include <QWidget>

namespace Ui {
class RecoveryFailure;
}

class RecoveryFailureView : public QDialog {
  Q_OBJECT

public:
  explicit RecoveryFailureView(QWidget *parent = 0,
                               ProjectRecoveryPresenter *presenter = nullptr);
  ~RecoveryFailureView();
  void reject() override;

  void setProgressBarMaximum(int newValue);
  void connectProgressBar();
  void emitAbortScript();
  void changeStartMantidButton(const QString &string);

signals:
  void abortProjectRecoveryScript();

public slots:
  void updateProgressBar(int newValue, bool err);

private slots:
  void onClickLastCheckpoint();
  void onClickSelectedCheckpoint();
  void onClickOpenSelectedInScriptWindow();
  void onClickStartMantidNormally();

private:
  void addDataToTable(Ui::RecoveryFailure *ui);

  Ui::RecoveryFailure *ui;
  ProjectRecoveryPresenter *m_presenter;
};

#endif // RECOVERYFAILUREVIEW_H
