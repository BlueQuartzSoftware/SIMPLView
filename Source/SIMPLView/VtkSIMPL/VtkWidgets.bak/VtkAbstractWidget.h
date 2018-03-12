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

#ifndef _VtkAbstractWidget_h_
#define _VtkAbstractWidget_h_

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#endif

#include <QtWidgets/QWidget>
#include <vector>
#include <vtkSmartPointer.h>

class vtkRenderWindowInteractor;
class vtkImplicitFunction;

class VtkAbstractWidget : public QWidget
{
  Q_OBJECT

public:
  VtkAbstractWidget(QWidget* parent, double bounds[6], vtkRenderWindowInteractor* iren);
  ~VtkAbstractWidget();

  void getBounds(double bounds[6]);
  void getOrigin(double origin[3]);

  void setBounds(double bounds[6]);
  virtual void setOrigin(double origin[3]);
  virtual void setOrigin(double x, double y, double z);

  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void apply() = 0;
  virtual void reset() = 0;

  virtual vtkSmartPointer<vtkImplicitFunction> getImplicitFunction() = 0;

signals:
  void modified();

protected:
  virtual void updateBounds();
  virtual void updateOrigin();

  double bounds[6];
  double origin[3];

  const double MIN_SIZE = 6.0;

  vtkRenderWindowInteractor* m_renderWindowInteractor;

private:
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif /* _VtkAbstractWidget_h_  */
