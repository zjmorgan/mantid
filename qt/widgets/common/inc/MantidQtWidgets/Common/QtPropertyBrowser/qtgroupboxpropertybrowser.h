/****************************************************************************
**
** This file is part of a Qt Solutions component.
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

#include "qtpropertybrowser.h"
#include <QMap>

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtGroupBoxPropertyBrowserPrivate;
class QLabel;
class QGroupBox;
class QGridLayout;
class QFrame;

class EXPORT_OPT_MANTIDQT_COMMON QtGroupBoxPropertyBrowser : public QtAbstractPropertyBrowser {
  Q_OBJECT
public:
  QtGroupBoxPropertyBrowser(QWidget *parent = nullptr);
  ~QtGroupBoxPropertyBrowser() override;

protected:
  void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) override;
  void itemRemoved(QtBrowserItem *item) override;
  void itemChanged(QtBrowserItem *item) override;

private:
  QtGroupBoxPropertyBrowserPrivate *d_ptr;
  Q_DECLARE_PRIVATE(QtGroupBoxPropertyBrowser)
  Q_DISABLE_COPY(QtGroupBoxPropertyBrowser)
  Q_PRIVATE_SLOT(d_func(), void slotUpdate())
  Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed())
};

class QtGroupBoxPropertyBrowserPrivate {
  QtGroupBoxPropertyBrowser *q_ptr;
  Q_DECLARE_PUBLIC(QtGroupBoxPropertyBrowser)
public:
  void init(QWidget *parent);

  void propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex);
  void propertyRemoved(QtBrowserItem *index);
  void propertyChanged(QtBrowserItem *index);
  QWidget *createEditor(QtProperty *property, QWidget *parent) const { return q_ptr->createEditor(property, parent); }

  void slotEditorDestroyed();
  void slotUpdate();

  struct WidgetItem {
    WidgetItem()
        : widget(nullptr), label(nullptr), widgetLabel(nullptr), groupBox(nullptr), layout(nullptr), line(nullptr),
          parent(nullptr) {}
    QWidget *widget; // can be null
    QLabel *label;
    QLabel *widgetLabel;
    QGroupBox *groupBox;
    QGridLayout *layout;
    QFrame *line;
    WidgetItem *parent;
    QList<WidgetItem *> children;
  };

private:
  void updateLater();
  void updateItem(WidgetItem *item);
  void insertRow(QGridLayout *layout, int row) const;
  void removeRow(QGridLayout *layout, int row) const;

  bool hasHeader(WidgetItem *item) const;

  QMap<QtBrowserItem *, WidgetItem *> m_indexToItem;
  QMap<WidgetItem *, QtBrowserItem *> m_itemToIndex;
  QMap<QWidget *, WidgetItem *> m_widgetToItem;
  QGridLayout *m_mainLayout;
  QList<WidgetItem *> m_children;
  QList<WidgetItem *> m_recreateQueue;
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif
