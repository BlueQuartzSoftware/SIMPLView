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

#ifndef _vtkDataSetFilter_h_
#define _vtkDataSetFilter_h_



#include <vtkSmartPointer.h>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "ui_VtkDataSetFilter.h"

// Vtk Forward Declarations
class vtkTrivialProducer;
class vtkAlgorithmOutput;

// VtkSIMPL Forward Declarations
class SVVtkDataSetRenderViewConf;


/**
 * @brief The VtkDataSetFilter class
 */
class VtkDataSetFilter : public VtkAbstractFilter, private Ui::VtkDataSetFilter
{
  Q_OBJECT

public:
  /**
  * @brief Constuctor
  * @param parentWidget
  * @param dataSetStruct
  */
  VtkDataSetFilter(QWidget* parentWidget, SVVtkDataSetRenderViewConf* dataSetStruct, vtkRenderWindowInteractor* iren);

  /**
  * @brief Deconstructor
  */
  ~VtkDataSetFilter();

  /**
  * @brief Initializes the trivial producer and connects it to the vtkMapper
  */
  void setFilter() override;

  /**
  * @brief Returns the data set bounds
  * @return
  */
  double* getBounds() override;

  /**
  * @brief Sets the filter's bounds
  * @param bounds
  */
  void setBounds(double* bounds) override;

  /**
  * @brief Sets the input vtkDataSet
  * @param inputData
  */
  void setInputData(vtkSmartPointer<vtkDataSet> inputData) override;

  /**
  * @brief Calculates the output data
  */
  void calculateOutput() override;

  /**
  * @brief Returns the filter's name
  * @return
  */
  const QString getFilterName() override;

  /**
  * @brief Returns whether or not the filter can be deleted. 
  * VtkDataSetFilters cannot be deleted.
  * @return
  */
  bool canDelete() override;

  /**
  * @brief Returns the output data type for the filter
  * @return
  */
  dataType_t getOutputType() override;

  /**
  * @brief Returns the required input data type
  * @return
  */
  static dataType_t getRequiredInputType();

  /**
  * @brief Returns the VtkDataSetStruct_t used by the filter
  * @return
  */
  SVVtkDataSetRenderViewConf* getDataSetStruct() override;

private:

  SVVtkDataSetRenderViewConf*    m_dataSetStruct;
  QString                        m_dataSetName;

  vtkSmartPointer<vtkTrivialProducer> m_trivialProducer;

  //		int m_scalarSetId;
};

#endif /* _vtkDataSetFilter_h_ */
