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

#include "VtkThresholdFilter.h"

#include <QtCore/QString>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractGeometry.h>
#include <vtkExtractSelectedThresholds.h>
#include <vtkExtractUnstructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImplicitDataSet.h>
#include <vtkMergeFilter.h>
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkStructuredPoints.h>
#include <vtkThreshold.h>
#include <vtkTrivialProducer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"

#include "VtkSIMPL/VtkWidgets/VtkThresholdWidget.h"

#include "VtkFilters/moc_VtkThresholdFilter.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkThresholdFilter::VtkThresholdFilter(QWidget* parentWidget, VtkAbstractFilter* parent)
: VtkAbstractFilter(parentWidget, parent->getInteractor())
{
  setupUi(this);

  m_thresholdAlgorithm = nullptr;
  setParentFilter(parent);

  vtkSmartPointer<vtkDataArray> dataArray = getBaseDataArray(parent->getViewScalarId());
  double range[2] = {dataArray->GetRange()[0], dataArray->GetRange()[1]};

  m_thresholdWidget = new VtkThresholdWidget(thresholdFunctionWidget, range, parent->getBounds(), parent->getInteractor());
  m_thresholdWidget->show();

  connect(m_thresholdWidget, SIGNAL(modified()), this, SLOT(changesWaiting()));

  setFilter();

  setupScalarsComboBox();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkThresholdFilter::~VtkThresholdFilter()
{
  delete m_thresholdWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::setBounds(double* bounds)
{
  if(nullptr == bounds)
  {
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::setFilter()
{
  m_currentId = m_parentFilter->getViewScalarId();
  m_lastId = m_currentId;

  m_thresholdAlgorithm = vtkSmartPointer<vtkThreshold>::New();

  if(nullptr != m_parentFilter)
  {
    m_ParentProducer->SetOutput(m_parentFilter->getOutput());
  }

  m_thresholdAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
  vtkSmartPointer<vtkDataArray> dataArray = m_dataSet->GetCellData()->GetScalars();
  m_filterMapper->SetScalarRange(dataArray->GetRange()[0], dataArray->GetRange()[1]);

  setThresholdScalarId(m_currentId);

  m_thresholdWidget->setLowerThreshold(dataArray->GetRange()[0]);
  m_thresholdWidget->setUpperThreshold(dataArray->GetRange()[1]);

  m_filterMapper->SetInputConnection(m_ParentProducer->GetOutputPort());
  setViewScalarId(m_parentFilter->getViewScalarId());

  m_ConnectedInput = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::setInputData(vtkSmartPointer<vtkDataSet> inputData)
{
  if(nullptr == inputData.GetPointer())
  {
    return;
  }

  if(nullptr == m_thresholdAlgorithm)
  {
    return;
  }

  m_thresholdWidget->setBounds(inputData->GetBounds());
  m_ParentProducer->SetOutput(inputData);
  m_ConnectedInput = false;

  m_dataSet = inputData;

  setDirty();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::calculateOutput()
{
  if(!m_ConnectedInput && m_parentFilter)
  {
    m_thresholdAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
    m_ConnectedInput = true;

    m_filterMapper->SetInputConnection(m_thresholdAlgorithm->GetOutputPort());
  }

  vtkDataSet* input = m_parentFilter->getOutput();
  input->GetCellData()->SetActiveScalars(scalarIdToName(m_currentId));

  m_thresholdAlgorithm->SetInputData(input);

  m_thresholdAlgorithm->ThresholdBetween(m_thresholdWidget->getLowerBound(), m_thresholdWidget->getUpperBound());
  m_thresholdAlgorithm->Update();
  m_dataSet = m_thresholdAlgorithm->GetOutput();

  m_dataSet->GetCellData()->SetActiveScalars(scalarIdToName(getViewScalarId()));

  updateMapperScalars();
  m_isDirty = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VtkThresholdFilter::getFilterName()
{
  return "Threshold";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractWidget* VtkThresholdFilter::getWidget()
{
  return m_thresholdWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::apply()
{
  m_thresholdWidget->apply();
  m_lastId = m_currentId;

  setDirty();
  refresh();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::reset()
{
  scalarsComboBox->setCurrentIndex(m_lastId);
  setThresholdScalarId(m_lastId);

  m_thresholdWidget->reset();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::setThresholdScalarId(int id)
{
  if(nullptr == m_dataSet)
  {
    return;
  }
  if(id < 0)
  {
    id = 0;
  }

  m_currentId = id;

  vtkDataArray* dataArray = getBaseDataArray(id);

  if(nullptr != dataArray)
  {
    m_thresholdWidget->setScalarRange(dataArray->GetRange()[0], dataArray->GetRange()[1]);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdFilter::setupScalarsComboBox()
{
  if(nullptr == m_dataSet)
  {
    return;
  }

  QList<QString> scalarNameList;
  vtkCellData* cellData = m_dataSet->GetCellData();

  for(int i = 0; i < cellData->GetNumberOfArrays(); i++)
  {
    scalarNameList.push_back(cellData->GetArray(i)->GetName());
  }

  scalarsComboBox->clear();
  scalarsComboBox->addItems(scalarNameList);

  if(scalarNameList.length() > 0)
  {
    scalarsComboBox->setCurrentIndex(0);
  }

  // set scalar set to threshold against
  connect(scalarsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setThresholdScalarId(int)));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkThresholdFilter::getOutputType()
{
  return UNSTRUCTURED_GRID;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkThresholdFilter::getRequiredInputType()
{
  return ANY_DATA_SET;
}
