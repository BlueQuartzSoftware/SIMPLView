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

#ifndef _VtkLookupTableController_h_
#define _VtkLookupTableController_h_

#include <vector>

#include <vtkSmartPointer.h>

class vtkColorTransferFunction;
class QJsonObject;

class VtkLookupTableController
{
public:
  struct RgbPos_t
  {
    double x, r, g, b;
  };

  VtkLookupTableController();
  ~VtkLookupTableController();

  void createColorTransferFunction();
  vtkSmartPointer<vtkColorTransferFunction> getColorTransferFunction();
  void setColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

  void parseRgbJson(const QJsonObject& json);

  void invert();

  void normalizePositions();

  double* getRange();
  void setRange(double range[2]);
  void setRange(double min, double max);

  void addRgbPoint(double x, double r, double g, double b);
  void addRgbPoint(double x, double rgb[3]);
  void addRgbPoint(RgbPos_t rgbPos);

  int getNumberOfRgbPoints();

  double* getRgbColor(int index);
  void setColor(int index, double r, double g, double b);
  void setColor(int index, double rgb[3]);

  double getRgbPosition(int index);
  void setRgbPositon(int index, double x);

  void removeRgbPoint(int index);

  bool equals(VtkLookupTableController* other);

protected:
  void update();

  std::vector<RgbPos_t> m_baseRgbPosVector;
  std::vector<RgbPos_t> m_rgbPosVector;
  static std::vector<RgbPos_t> m_defaultRgbPosVector;

  double m_range[2];
  double m_baseRange[2];

  vtkSmartPointer<vtkColorTransferFunction> m_colorTransferFunction;
};

#endif /* _VtkLookupTableController_h_ */
