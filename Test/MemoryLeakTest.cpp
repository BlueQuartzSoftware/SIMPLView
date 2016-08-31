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

#include <QtCore/QCoreApplication>

#include <QtWidgets/QVBoxLayout>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/Utilities/UnitTestSupport.hpp"
#include "SIMPLib/Utilities/QMetaObjectUtilities.h"

#include "SIMPLib/CoreFilters/CreateDataContainer.h"
#include "SIMPLib/CoreFilters/CreateAttributeMatrix.h"
#include "SIMPLib/CoreFilters/CreateDataArray.h"

#include "SVWidgetsLib/Widgets/SVPipelineFilterWidget.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setObjectName(QString::fromUtf8("layout"));
  layout->setContentsMargins(2, 2, 2, 2);
  layout->setSpacing(3);

  for (int i = 0; i < 100; i++)
  //while (true)
  {
    // Add widgets
    {
      CreateDataContainer::Pointer filter = CreateDataContainer::New();
      SVPipelineFilterWidget* filterWidget = new SVPipelineFilterWidget(filter);
      layout->insertWidget(0, filterWidget);
    }

      {
        CreateAttributeMatrix::Pointer filter = CreateAttributeMatrix::New();
        SVPipelineFilterWidget* filterWidget = new SVPipelineFilterWidget(filter);
        layout->insertWidget(1, filterWidget);
      }

      {
        CreateDataArray::Pointer filter = CreateDataArray::New();
        SVPipelineFilterWidget* filterWidget = new SVPipelineFilterWidget(filter);
        layout->insertWidget(2, filterWidget);
      }

      QSpacerItem* verticalSpacer = new QSpacerItem(20, 361, QSizePolicy::Minimum, QSizePolicy::Expanding);
      layout->insertSpacerItem(-1, verticalSpacer);

      // Remove widgets
      {
        QWidget* widget = layout->itemAt(2)->widget();
        if (widget != nullptr)
        {
          layout->removeWidget(widget);
          delete widget;
        }
      }

      {
        QWidget* widget = layout->itemAt(1)->widget();
        if (widget != nullptr)
        {
          layout->removeWidget(widget);
          delete widget;
        }
      }

      {
        QWidget* widget = layout->itemAt(0)->widget();
        if (widget != nullptr)
        {
          layout->removeWidget(widget);
          delete widget;
        }
      }

      layout->removeItem(verticalSpacer);
      delete verticalSpacer;
  }

  return app.exec();
}

