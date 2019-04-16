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

#include "SIMPLViewUIMessageHandler.h"

#include "SIMPLib/Messages/FilterProgressMessage.h"
#include "SIMPLib/Messages/FilterStatusMessage.h"
#include "SIMPLib/Messages/PipelineProgressMessage.h"
#include "SIMPLib/Messages/PipelineStatusMessage.h"

#include "SIMPLView/SIMPLView_UI.h"
#include "SVWidgetsLib/Widgets/SVOverlayWidgetButton.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLViewUIMessageHandler::SIMPLViewUIMessageHandler(SIMPLView_UI* uiWidget)
: m_UIWidget(uiWidget)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewUIMessageHandler::processMessage(const FilterStatusMessage* msg) const
{
  QString statusMessage = msg->generateMessageString();

  if(nullptr != m_UIWidget->statusBar())
  {
    m_UIWidget->statusBar()->showMessage(statusMessage);
  }

  statusMessage.prepend("      ");
  appendStatusMessageToPipelineOutput(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewUIMessageHandler::processMessage(const PipelineProgressMessage* msg) const
{
  float progValue = static_cast<float>(msg->getProgressValue()) / 100;
  m_UIWidget->m_Ui->pipelineListWidget->setProgressValue(progValue);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewUIMessageHandler::processMessage(const PipelineStatusMessage* msg) const
{
  QString statusMessage = msg->generateMessageString();

  if(nullptr != m_UIWidget->statusBar())
  {
    m_UIWidget->statusBar()->showMessage(statusMessage);
  }

  appendStatusMessageToPipelineOutput(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewUIMessageHandler::appendStatusMessageToPipelineOutput(const QString& statusMessage) const
{
  // Allow status messages to open the standard output widget
  if(SIMPLView::DockWidgetSettings::HideDockSetting::OnStatusAndError == StandardOutputWidget::GetHideDockSetting())
  {
    if(!m_UIWidget->m_IssuesOverlayBtn->isChecked())
    {
      m_UIWidget->m_IssuesOverlayBtn->setChecked(true);
    }
  }

  // Allow status messages to open the issuesDockWidget as well
  if(SIMPLView::DockWidgetSettings::HideDockSetting::OnStatusAndError == IssuesWidget::GetHideDockSetting())
  {
    if(!m_UIWidget->m_FilterInputOverlayBtn->isChecked())
    {
      m_UIWidget->m_IssuesOverlayBtn->setChecked(true);
    }
  }

  //  QString text;
  //  QTextStream ts(&text);
  //  ts << "<a style=\"color: " << SVStyle::Instance()->getQLabel_color().name(QColor::HexRgb) << ";\" >" << statusMessage << "</span>";

  m_UIWidget->m_IssuesUi->stdOutWidget->appendText(statusMessage);
}
