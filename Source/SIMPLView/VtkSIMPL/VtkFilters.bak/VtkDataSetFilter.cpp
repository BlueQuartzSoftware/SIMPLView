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

#include "VtkDataSetFilter.h"

#include <vtkAlgorithmOutput.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkTrivialProducer.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include "VtkSIMPL/VtkSupport/SIMPLVtkBridge.h"
#include "VtkSIMPL/VtkSupport/VtkLookupTableController.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h"


#include "VtkFilters/moc_VtkDataSetFilter.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkDataSetFilter::VtkDataSetFilter(QWidget* parentWidget, SVVtkDataSetRenderViewConf* dataSetStruct, vtkRenderWindowInteractor* iren)
: VtkAbstractFilter(parentWidget, iren)
{
  setupUi(this);

  m_dataSetName = QString::fromStdString(dataSetStruct->getName());
  m_dataSetStruct = dataSetStruct;
  m_dataSet = dataSetStruct->getDataSet();
  m_changesWaiting = false;

  setFilter();
  setViewScalarId(0);
  vtkDataArray* dataArray = m_dataSetStruct->getDataSet()->GetCellData()->GetArray(getViewScalarId());
  if(dataArray)
  {
    m_lookupTable->setRange(dataArray->GetRange());
  }

  hide();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkDataSetFilter::~VtkDataSetFilter()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double* VtkDataSetFilter::getBounds()
{
  return m_dataSet->GetBounds();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkDataSetFilter::setBounds(double* bounds)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkDataSetFilter::setInputData(vtkSmartPointer<vtkDataSet> inputData)
{
  m_dataSet = inputData;
  m_dataSet->ComputeBounds();

  if(m_trivialProducer != nullptr)
  {
    m_trivialProducer->SetOutput(m_dataSet);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkDataSetFilter::setFilter()
{
  m_dataSetStruct->getDataSet()->ComputeBounds();
  m_dataSet->ComputeBounds();
  
  vtkCellData* cellData = m_dataSet->GetCellData();
  if(cellData)
  {
    vtkDataArray* dataArray = cellData->GetArray(0);
    if(dataArray)
    {
      char* name = dataArray->GetName();
      cellData->SetActiveScalars(name);
    }
    
  }
  
  m_trivialProducer = vtkSmartPointer<vtkTrivialProducer>::New();
  m_trivialProducer->SetOutput(m_dataSet);

  m_filterMapper->SetInputConnection(m_trivialProducer->GetOutputPort());

  vtkSmartPointer<vtkDataArray> colorArray = m_dataSet->GetCellData()->GetScalars();
  if(colorArray != nullptr)
  {
    m_filterMapper->SetScalarRange(colorArray->GetRange()[0], colorArray->GetRange()[1]);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkDataSetFilter::calculateOutput()
{
  m_dataSet->ComputeBounds();
  m_isDirty = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VtkDataSetFilter::getFilterName()
{
  return m_dataSetName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkDataSetFilter::canDelete()
{
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataSetRenderViewConf *VtkDataSetFilter::getDataSetStruct()
{
  return m_dataSetStruct;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkDataSetFilter::getOutputType()
{
  return IMAGE_DATA;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkDataSetFilter::getRequiredInputType()
{
  return ANY_DATA_SET;
}
