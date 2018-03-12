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

#include "VtkClipFilter.h"

#include <QApplication>
#include <QString>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTableBasedClipDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkFilters/moc_VtkClipFilter.cpp"
#include "VtkSIMPL/VtkWidgets/VtkBoxWidget.h"
#include "VtkSIMPL/VtkWidgets/VtkPlaneWidget.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkClipFilter::VtkClipFilter(QWidget* parentWidget, VtkAbstractFilter* parent)
: VtkAbstractFilter(parentWidget, parent->getInteractor())
{
  setupUi(this);

  m_clipAlgorithm = nullptr;
  setParentFilter(parent);

  m_planeWidget = new VtkPlaneWidget(clipFunctionWidget, parent->getBounds(), parent->getInteractor());
  m_boxWidget = new VtkBoxWidget(clipFunctionWidget, parent->getBounds(), parent->getInteractor());

  connect(clipTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeClipType(int)));
  connect(insideOutCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setInsideOut(int)));

  connect(m_planeWidget, SIGNAL(modified()), this, SLOT(changesWaiting()));
  connect(m_boxWidget, SIGNAL(modified()), this, SLOT(changesWaiting()));

  setFilter();

  m_lastInsideOutState = 0;
  changeClipType(PLANE, false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkClipFilter::~VtkClipFilter()
{
  m_clipAlgorithm = nullptr;
  delete m_planeWidget;
  delete m_boxWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::setBounds(double* bounds)
{
  if(nullptr == bounds)
  {
    return;
  }

  m_planeWidget->setBounds(bounds);
  m_boxWidget->setBounds(bounds);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::setFilter()
{
  m_clipAlgorithm = vtkSmartPointer<vtkTableBasedClipDataSet>::New();
  m_clipAlgorithm->SetClipFunction(getWidget()->getImplicitFunction());

  if(nullptr != m_parentFilter)
  {
    m_ParentProducer->SetOutput(m_parentFilter->getOutput());
  }

  m_clipAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
  m_filterMapper->SetInputConnection(m_ParentProducer->GetOutputPort());
  setViewScalarId(m_parentFilter->getViewScalarId());

  m_ConnectedInput = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::setInputData(vtkSmartPointer<vtkDataSet> inputData)
{
  if(nullptr == inputData.GetPointer())
  {
    return;
  }

  m_dataSet = inputData;

  if(nullptr == m_clipAlgorithm)
  {
    return;
  }

  m_planeWidget->setBounds(inputData->GetBounds());
  m_boxWidget->setBounds(inputData->GetBounds());

  m_ConnectedInput = false;

  setDirty();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::calculateOutput()
{
  if(!m_ConnectedInput && m_parentFilter)
  {
    m_clipAlgorithm->SetInputData(m_parentFilter->getOutput());
    m_ConnectedInput = true;

    m_filterMapper->SetInputConnection(m_clipAlgorithm->GetOutputPort());
  }

  m_clipAlgorithm->Update();
  m_dataSet = m_clipAlgorithm->GetOutput();
  m_dataSet->ComputeBounds();

  updateMapperScalars();
  m_isDirty = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VtkClipFilter::getFilterName()
{
  return "Clip";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractWidget* VtkClipFilter::getWidget()
{
  switch(m_currentClipType)
  {
  case PLANE:
    return m_planeWidget;
  case BOX:
    return m_boxWidget;
  default:
    return m_planeWidget;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::apply()
{
  m_lastClipType = m_currentClipType;
  m_lastInsideOutState = m_clipAlgorithm->GetInsideOut();

  VtkAbstractWidget* currentWidget = getWidget();
  currentWidget->apply();
  m_clipAlgorithm->SetClipFunction(currentWidget->getImplicitFunction());

  setDirty();
  refresh();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::reset()
{
  insideOutCheckBox->setChecked(m_lastInsideOutState);

  m_planeWidget->reset();
  m_boxWidget->reset();

  clipTypeComboBox->setCurrentIndex(m_lastClipType);
  changeClipType(m_lastClipType);

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::changeClipType(int type, bool shouldRepaint)
{
  m_currentClipType = (clipType_t)type;

  switch(m_currentClipType)
  {
  case PLANE:
    m_planeWidget->show();
    m_planeWidget->enable();

    m_boxWidget->hide();
    m_boxWidget->disable();

    clipFunctionWidget->setMinimumHeight(m_planeWidget->minimumHeight());
    break;
  case BOX:
    m_planeWidget->hide();
    m_planeWidget->disable();

    m_boxWidget->show();
    m_boxWidget->enable();

    clipFunctionWidget->setMinimumHeight(m_boxWidget->minimumHeight());
    break;
  default:
    m_planeWidget->show();
    m_planeWidget->enable();

    m_boxWidget->hide();
    m_boxWidget->disable();

    clipFunctionWidget->setMinimumHeight(m_planeWidget->minimumHeight());
  }

  getInteractor()->Render();

  update();
  changesWaiting();

  if(shouldRepaint)
  {
    repaint();
    QApplication::instance()->processEvents();

    emit resized(shouldRepaint);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkClipFilter::setInsideOut(int state)
{
  m_clipAlgorithm->SetInsideOut(state);

  changesWaiting();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkClipFilter::getOutputType()
{
  return UNSTRUCTURED_GRID;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkClipFilter::getRequiredInputType()
{
  return ANY_DATA_SET;
}
