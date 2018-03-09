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

#ifndef _VtkToggleWidget_h_
#define _VtkToggleWidget_h_

#include "ui_VtkToggleWidget.h"
#include <QWidget>

class QString;

/**
 * @class VtkToggleWidget VtkToggleWidget.h 
 * SIMPLView/VtkSIMPL/QtWidgets/VtkToggleWidget.h
 * @brief This class handles a togglable contents widget where the user can 
 * show or hide it by clicking on a button.
 */
class VtkToggleWidget : public QWidget, private Ui::VtkToggleWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  */
  VtkToggleWidget(QWidget* parent);

  /**
  * @brief Deconstructor
  */
  ~VtkToggleWidget();

  /**
  * @brief Sets the title used by the button
  * @param title
  */
  void setTitle(QString title);

  /**
  * @brief Returns the title used by the button
  * @return
  */
  QString getTitle();

  /**
  * @brief Sets the widget being shown or hidden
  * @param widget
  */
  void setWidget(QWidget* widget);

  /**
  * @brief Returns the widget being shown or hidden
  * @return
  */
  QWidget* getWidget();

  /**
  * @brief Returns true if the contents widget is visible
  * @return
  */
  bool isContentVisible();

  /**
  * @brief Sets whether or not the contents widget is visible
  * @param visible
  */
  void setContentVisible(bool visible);

public slots:
  /**
  * @brief Toggles whether or not the contents widget is visible
  */
  void toggleVisibility();

signals:
  /**
  * @brief Signals that the widget 
  */
  void resized();

protected slots:
  /**
  * @brief Resets the size of the widget and repaints it if necessary.
  * @param shouldRepaint
  */
  void resetSize(bool shouldRepaint = false);

private:
  QString m_title;
  QWidget* m_widget;

  bool m_show;
};

#endif /* _VtkToggleWidget_h_ */
