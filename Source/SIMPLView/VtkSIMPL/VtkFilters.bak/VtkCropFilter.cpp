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

#include "VtkCropFilter.h"

#include <QString>

#include <vtkActor.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkWidgets/VtkCropWidget.h"
#include <vtkExtractVOI.h>

#include <vtkRenderWindowInteractor.h>

#include "VtkFilters/moc_VtkCropFilter.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkCropFilter::VtkCropFilter(QWidget* parentWidget, VtkAbstractFilter* parent)
: VtkAbstractFilter(parentWidget, parent->getInteractor())
{
  setupUi(this);

  m_cropAlgorithm = nullptr;
  setParentFilter(parent);

  m_cropWidget = new VtkCropWidget(cropFunctionWidget, vtkImageData::SafeDownCast(parent->getOutput()), parent->getInteractor());
  m_cropWidget->show();

  connect(m_cropWidget, SIGNAL(modified()), this, SLOT(changesWaiting()));

  setFilter();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkCropFilter::~VtkCropFilter()
{
  m_cropAlgorithm = nullptr;
  delete m_cropWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropFilter::setBounds(double* bounds)
{
  if(nullptr == bounds)
  {
    return;
  }

  m_cropWidget->setBounds(bounds);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropFilter::setFilter()
{
  m_cropAlgorithm = vtkSmartPointer<vtkExtractVOI>::New();
  m_cropAlgorithm->IncludeBoundaryOn();
  m_cropAlgorithm->SetVOI(m_cropWidget->getVOI());

  if(nullptr != m_parentFilter)
  {
    m_ParentProducer->SetOutput(m_parentFilter->getOutput());
  }

  m_cropAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
  m_filterMapper->SetInputConnection(m_ParentProducer->GetOutputPort());
  setViewScalarId(m_parentFilter->getViewScalarId());

  m_ConnectedInput = false;

  vtkSmartPointer<vtkDataArray> dataArray = getScalarSet();
  m_filterMapper->SetScalarRange(dataArray->GetRange()[0], dataArray->GetRange()[1]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropFilter::setInputData(vtkSmartPointer<vtkDataSet> inputData)
{
  if(nullptr == inputData.GetPointer())
  {
    return;
  }

  if(nullptr == m_cropAlgorithm)
  {
    return;
  }

  m_ConnectedInput = false;

  m_ParentProducer->SetOutput(inputData);

  setDirty();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropFilter::calculateOutput()
{
  if(!m_ConnectedInput && m_parentFilter)
  {
    m_cropAlgorithm->SetInputConnection(m_ParentProducer->GetOutputPort());
    m_ConnectedInput = true;

    m_filterMapper->SetInputConnection(m_cropAlgorithm->GetOutputPort());
  }

  int* voi = m_cropWidget->getVOI();
  int* sampleRate = m_cropWidget->getSampleRate();

  m_cropAlgorithm->SetVOI(voi);
  m_cropAlgorithm->SetSampleRate(sampleRate);
  m_cropAlgorithm->GetOutput()->SetDimensions(voi[1], voi[3], voi[5]);

  m_cropAlgorithm->Update();
  m_dataSet = m_cropAlgorithm->GetOutput();
  m_dataSet->ComputeBounds();

  updateMapperScalars();

  m_isDirty = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VtkCropFilter::getFilterName()
{
  return "Crop";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractWidget* VtkCropFilter::getWidget()
{
  return m_cropWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropFilter::apply()
{
  m_cropWidget->apply();

  setDirty();
  refresh();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropFilter::reset()
{
  m_cropWidget->reset();

  m_changesWaiting = false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkCropFilter::getOutputType()
{
  return IMAGE_DATA;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::dataType_t VtkCropFilter::getRequiredInputType()
{
  return IMAGE_DATA;
}
