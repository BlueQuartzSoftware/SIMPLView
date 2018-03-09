/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "VtkToggleWidget.h"

#include <QApplication>
#include <QPushButton>
#include <QSize>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "VtkSIMPL/QtWidgets/moc_VtkToggleWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkToggleWidget::VtkToggleWidget(QWidget* parent)
: QWidget(parent)
, m_widget(nullptr)
, m_show(true)
{
  setupUi(this);
  setAttribute(Qt::WA_LayoutUsesWidgetRect);

  verticalLayout->setAlignment(Qt::AlignTop);

  m_title = toggleButton->text();

  connect(toggleButton, SIGNAL(clicked()), this, SLOT(toggleVisibility()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkToggleWidget::~VtkToggleWidget()
{
  disconnect(toggleButton, SIGNAL(clicked()), this, SLOT(toggleVisibility()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkToggleWidget::setTitle(QString title)
{
  toggleButton->setText(title);
  toggleButton->update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VtkToggleWidget::getTitle()
{
  return toggleButton->text();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkToggleWidget::setWidget(QWidget* widget)
{
  if(m_widget)
  {
    m_widget->hide();
    m_widget->setParent(nullptr);

    VtkAbstractFilter* filter = dynamic_cast<VtkAbstractFilter*>(m_widget);
    if(filter)
    {
      disconnect(filter, SIGNAL(resized(bool)), this, SLOT(resetSize(bool)));
    }
  }

  m_widget = widget;
  if(m_widget)
  {
    verticalLayout->addWidget(m_widget);
    verticalLayout->update();

    if(m_show)
    {
      m_widget->show();
    }
    else
    {
      m_widget->hide();
    }

    m_widget->update();

    VtkAbstractFilter* filter = dynamic_cast<VtkAbstractFilter*>(m_widget);
    if(filter)
    {
      connect(filter, SIGNAL(resized(bool)), this, SLOT(resetSize(bool)));
    }
  }

  update();
  resetSize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QWidget* VtkToggleWidget::getWidget()
{
  return m_widget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkToggleWidget::isContentVisible()
{
  if(nullptr == m_widget)
  {
    return false;
  }

  return m_widget->isVisible();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkToggleWidget::setContentVisible(bool visible)
{
  if(nullptr == m_widget)
  {
    return;
  }

  m_show = visible;

  if(nullptr == m_widget)
  {
    return;
  }

  if(visible)
  {
    m_widget->show();
  }
  else
  {
    m_widget->hide();
  }

  update();
  resetSize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkToggleWidget::toggleVisibility()
{
  setContentVisible(!m_show);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkToggleWidget::resetSize(bool shouldRepaint)
{
  int widgetHeight = 0;
  if(m_widget != nullptr && m_show)
  {
    widgetHeight = m_widget->sizeHint().height();
  }

  QSize newSize;
  newSize.setWidth(size().width());
  newSize.setHeight(toggleButton->size().height() + line->size().height() + widgetHeight + verticalLayout->spacing() * 2);

  resize(newSize);

  if(shouldRepaint)
  {
    repaint();
    QApplication::instance()->processEvents();
  }

  emit resized();
}
