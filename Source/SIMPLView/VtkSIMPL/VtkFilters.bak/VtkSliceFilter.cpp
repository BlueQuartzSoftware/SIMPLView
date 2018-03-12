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

#include "VtkSliceFilter.h"

#include <QtCore/QString>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCutter.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkWidgets/VtkPlaneWidget.h"

#include "VtkFilters/moc_VtkSliceFilter.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkSliceFilter::VtkSliceFilter(QWidget* parentWidget, VtkAbstractFilter* parent)
: VtkAbstractFilter(parentWidget, parent->getInteractor())
{
  setupUi(this);

  m_sliceAlgorithm = nullptr;
  setParentFilter(parent);

  m_sliceWidget = new VtkPlaneWidget(sliceFunctionWidget, parent->getBounds(), parent->getInteractor());
  m_sliceWidget->show();

  connect(m_sliceWidget, SIGNAL(modified()), this, SLOT(changesWaiting()));

  setFilter();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkSliceFilter::~VtkSliceFilter()
{
  m_sliceAlgorithm = nullptr;
  delete m_sliceWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkSliceFilter::setBounds(double* bounds)
{
  if(nullptr == bounds)
  {
    return;
  }

  m_sliceWidget->setBounds(bounds);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkSliceFilter::setFilter()
{
  m_sliceAlgorithm = vtkSmartPointer<vtkCutter>::New();
  m_sliceAlgorithm->SetCutFunction(m_sliceWidget->getImplicitFunction());

  if(nullptr != m_parentFilter)
  {
    m_ParentProducer->SetOutput(m_parentFilter->getOutput());
  }

  m_sliceAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
  m_filterMapper->SetInputConnection(m_ParentProducer->GetOutputPort());
  setViewScalarId(m_parentFilter->getViewScalarId());

  //calculateOutput();
  m_ConnectedInput = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkSliceFilter::setInputData(vtkSmartPointer<vtkDataSet> inputData)
{
  if(nullptr == inputData.GetPointer())
  {
    return;
  }

  if(nullptr == m_sliceAlgorithm)
  {
    return;
  }

  m_sliceWidget->setBounds(inputData->GetBounds());
  m_ParentProducer->SetOutput(inputData);
  m_ConnectedInput = false;

  setDirty();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkSliceFilter::calculateOutput()
{
  if(!m_ConnectedInput && m_parentFilter)
  {
    m_sliceAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
    m_ConnectedInput = true;

    m_filterMapper->SetInputConnection(m_sliceAlgorithm->GetOutputPort());
  }

  m_sliceAlgorithm->SetCutFunction(m_sliceWidget->getImplicitFunction());
  m_sliceAlgorithm->Update();
  m_dataSet = m_sliceAlgorithm->GetOutput();
  m_dataSet->ComputeBounds();

  updateMapperScalars();
  m_isDirty = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VtkSliceFilter::getFilterName()
{
  return "Slice";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractWidget* VtkSliceFilter::getWidget()
{
  return m_sliceWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkSliceFilter::apply()
{
  m_sliceWidget->apply();

  setDirty();
  refresh();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkSliceFilter::reset()
{
  m_sliceWidget->reset();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkSliceFilter::getOutputType()
{
  return POLY_DATA;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkSliceFilter::getRequiredInputType()
{
  return ANY_DATA_SET;
}
