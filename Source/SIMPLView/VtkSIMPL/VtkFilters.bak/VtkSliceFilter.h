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
#ifndef _VtkSliceFilter_H_
#define _VtkSliceFilter_H_

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "ui_VtkSliceFilter.h"

#include <vtkPlane.h>

class vtkCutter;
class vtkImplicitPlaneWidget2;
class VtkPlaneWidget;

/**
 * @class VtkSliceFilter VtkSliceFilter.h 
 * SIMPLView/VtkSIMPL/VtkFilters/VtkSliceFilter.h
 * @brief This class controls the slice filter and, as with other classes 
 * inheriting from VtkAbstractFilter, can be chained together to further 
 * specify what part of the volume should be rendered.
 */
class VtkSliceFilter : public VtkAbstractFilter, private Ui::VtkSliceFilter
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parentWidget
  * @param parent
  */
  VtkSliceFilter(QWidget* parentWidget, VtkAbstractFilter* parent);

  /**
  * @brief Deconstructor
  */
  ~VtkSliceFilter();

  /**
  * @brief Sets the filter's bounds
  * @param bounds
  */
  void setBounds(double* bounds) override;

  /**
  * @brief Initializes the algorithm and connects it to the vtkMapper
  */
  void setFilter() override;

  /**
  * @brief Sets the input data for the algorithm
  * @param inputData
  */
  void setInputData(vtkSmartPointer<vtkDataSet> inputData) override;

  /**
  * @brief Calculates the output data for the algorithm
  */
  void calculateOutput() override;

  /**
  * @brief Returns the filter's name
  * @return
  */
  const QString getFilterName() override;

  /**
  * @brief Returns the VtkAbstractWidget used by the filter
  * @return
  */
  VtkAbstractWidget* getWidget() override;

  /**
  * @brief Applies the updated values to the algorithm and updates the output
  */
  void apply() override;

  /**
  * @brief Resets the filter to the last applied values
  */
  void reset() override;

  /**
  * @brief Returns the output data type
  * @return
  */
  dataType_t getOutputType() override;

  /**
  * @brief Returns the required input data type
  * @return
  */
  static dataType_t getRequiredInputType();

private:
  vtkSmartPointer<vtkPlane> m_slicePlane;
  vtkSmartPointer<vtkCutter> m_sliceAlgorithm;

  VtkPlaneWidget* m_sliceWidget;
};

#endif /* _VtkSliceFilter_H_ */
