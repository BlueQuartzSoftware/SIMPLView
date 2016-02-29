/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
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

#include "CalculatorWidget.h"

#include <QtCore/QMetaProperty>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSequentialAnimationGroup>

#include <QtWidgets/QMenu>

#include "SIMPLViewWidgetsLib/SIMPLViewWidgetsLibConstants.h"

#include "FilterParameterWidgetsDialogs.h"


// Include the MOC generated file for this class
#include "moc_CalculatorWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CalculatorWidget::CalculatorWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent) :
  FilterParameterWidget(parameter, filter, parent),
  m_ScalarsMenu(NULL),
  m_VectorsMenu(NULL)
{
  m_Filter = dynamic_cast<ArrayCalculator*>(filter);
  Q_ASSERT_X(m_Filter != NULL, "NULL Pointer", "CalculatorWidget can ONLY be used with an ArrayCalculator filter");

  m_FilterParameter = dynamic_cast<CalculatorFilterParameter*>(parameter);
  Q_ASSERT_X(m_FilterParameter != NULL, "NULL Pointer", "CalculatorWidget can ONLY be used with a CalculatorFilterParameter object");

  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CalculatorWidget::~CalculatorWidget()
{}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::setupGui()
{
  blockSignals(true);
  if (getFilterParameter() != NULL)
  {
    QString str = getFilter()->property(PROPERTY_NAME_AS_CHAR).toString();
    equation->setText(str);
  }
  blockSignals(false);

  // Catch when the filter is about to execute the preflight
  connect(getFilter(), SIGNAL(preflightAboutToExecute()),
          this, SLOT(beforePreflight()));

  // Catch when the filter is finished running the preflight
  connect(getFilter(), SIGNAL(preflightExecuted()),
          this, SLOT(afterPreflight()));

  // Catch when the filter wants its values updated
  connect(getFilter(), SIGNAL(updateFilterParameters(AbstractFilter*)),
          this, SLOT(filterNeedsInputParameters(AbstractFilter*)));

  connect(equation, SIGNAL(textChanged(const QString&)),
          this, SLOT(widgetChanged(const QString&)));

  connect(absBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(acosBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(addBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(asinBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(atanBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(ceilBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(cosBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(coshBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(divideBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(expBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(floorBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(iHatBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(jHatBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(kHatBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(leftBraceBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(lnBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(log10Btn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(magBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(multiplyBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(normBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(rightBraceBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(sinBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(sinhBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(sqrtBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(subtractBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(tanBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));
  connect(tanhBtn, SIGNAL(pressed()), this, SLOT(printButtonName()));

  applyChangesBtn->setVisible(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::printButtonName()
{
  QPushButton* button = static_cast<QPushButton*>(sender());
  if (NULL != button)
  {
    QString equationText = equation->text();
    equationText.append(button->text());
    equation->setText(equationText);
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::printActionName()
{
  QAction* action = static_cast<QAction*>(sender());
  if (NULL != action)
  {
    QString equationText = equation->text();
    equationText.append(action->text());
    equation->setText(equationText);
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_clearBtn_pressed()
{
  equation->clear();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_v1v2Btn_pressed()
{
  QString equationText = equation->text();
  equationText.append('.');
  equation->setText(equationText);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_xExpYBtn_pressed()
{
  QString equationText = equation->text();
  equationText.append('^');
  equation->setText(equationText);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_scalarsBtn_pressed()
{
  if (NULL != m_VectorsMenu)
  {
    delete m_ScalarsMenu;
    m_ScalarsMenu = NULL;
  }

  // Create Scalars Menu
  m_ScalarsMenu = new QMenu(this);

  AttributeMatrix::Pointer am = m_Filter->getDataContainerArray()->getAttributeMatrix(m_Filter->getSelectedAttributeMatrix());
  if (NULL == am)
  {
    return;
  }

  QStringList nameList = am->getAttributeArrayNames();

  for (int i = 0; i < nameList.size(); i++)
  {
    if (am->getAttributeArray(nameList[i])->getComponentDimensions()[0] == 1)
    {
      QAction* action = new QAction(nameList[i], m_ScalarsMenu);
      connect(action, SIGNAL(triggered()), this, SLOT(printActionName()));
      m_ScalarsMenu->addAction(action);
    }
  }

  scalarsBtn->setMenu(m_ScalarsMenu);

  scalarsBtn->showMenu();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_vectorsBtn_pressed()
{
  if (NULL != m_VectorsMenu)
  {
    delete m_VectorsMenu;
    m_VectorsMenu = NULL;
  }

  // Create Vectors Menu
  m_VectorsMenu = new QMenu(this);

  AttributeMatrix::Pointer am = m_Filter->getDataContainerArray()->getAttributeMatrix(m_Filter->getSelectedAttributeMatrix());
  if (NULL == am)
  {
    return;
  }

  QStringList nameList = am->getAttributeArrayNames();

  for (int i = 0; i < nameList.size(); i++)
  {
    if (am->getAttributeArray(nameList[i])->getComponentDimensions()[0] > 1)
    {
      QAction* action = new QAction(nameList[i], m_VectorsMenu);
      connect(action, SIGNAL(triggered()), this, SLOT(printActionName()));
      m_VectorsMenu->addAction(action);
    }
  }

  vectorsBtn->setMenu(m_VectorsMenu);

  vectorsBtn->showMenu();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_scalarsBtn_triggered(QAction *action)
{
  Q_UNUSED(action)

  on_scalarsBtn_pressed();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_vectorsBtn_triggered(QAction *action)
{
  Q_UNUSED(action)

  on_vectorsBtn_pressed();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_equation_returnPressed()
{
  //qDebug() << "DataArrayCreationWidget::on_value_returnPressed() " << this;
  m_DidCausePreflight = true;
  on_applyChangesBtn_clicked();
  m_DidCausePreflight = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::hideButton()
{
  equation->setToolTip("");
  applyChangesBtn->setVisible(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::widgetChanged(const QString& text)
{
  equation->setStyleSheet(QString::fromLatin1("color: rgb(255, 0, 0);"));
  equation->setToolTip("Press the 'Return' key to apply your changes");
  if(applyChangesBtn->isVisible() == false)
  {
    applyChangesBtn->setVisible(true);
    fadeInWidget(applyChangesBtn);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::beforePreflight()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::afterPreflight()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  bool ok = filter->setProperty(PROPERTY_NAME_AS_CHAR, equation->text());
  if (false == ok)
  {
    FilterParameterWidgetsDialogs::ShowCouldNotSetFilterParameter(getFilter(), getFilterParameter());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorWidget::on_applyChangesBtn_clicked()
{
  equation->setStyleSheet(QString(""));
  emit parametersChanged();

  QPointer<FaderWidget> faderWidget = new FaderWidget(applyChangesBtn);
  setFaderWidget(faderWidget);

  if (getFaderWidget())
  {
    faderWidget->close();
  }
  faderWidget = new FaderWidget(applyChangesBtn);
  faderWidget->setFadeOut();
  connect(faderWidget, SIGNAL(animationComplete() ),
          this, SLOT(hideButton()));
  faderWidget->start();
}

