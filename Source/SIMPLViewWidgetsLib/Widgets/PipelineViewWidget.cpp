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

#include "PipelineViewWidget.h"

#include <iostream>


#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include <QtCore/QMimeData>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <QtGui/QMouseEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDrag>
#include <QtGui/QClipboard>
#include <QtWidgets/QLabel>
#include <QtGui/QPixmap>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QMessageBox>

#include "Applications/SIMPLView/SIMPLViewApplication.h"
#include "Applications/SIMPLView/SIMPLViewMenuItems.h"

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/Common/PipelineMessage.h"
#include "SIMPLib/Common/FilterManager.h"
#include "SIMPLib/Common/IFilterFactory.hpp"
#include "SIMPLib/Common/FilterFactory.hpp"
#include "SIMPLib/FilterParameters/QFilterParametersReader.h"
#include "SIMPLib/FilterParameters/QFilterParametersWriter.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"

#include "QtSupportLib/QDroppableScrollArea.h"

#include "SIMPLViewWidgetsLib/FilterWidgetManager.h"
#include "SIMPLViewWidgetsLib/PipelineViewPtrMimeData.h"

#include "SIMPLViewWidgetsLib/FilterParameterWidgets/FilterParameterWidgetsDialogs.h"

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_PipelineViewWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewWidget::PipelineViewWidget(QWidget* parent) :
  QFrame(parent),
  m_ShiftStart(NULL),
  m_FilterWidgetLayout(NULL),
  m_FilterOrigPos(-1),
  m_DropBox(NULL),
  m_DropIndex(-1),
  m_EmptyPipelineLabel(NULL),
  m_ScrollArea(NULL),
  m_AutoScroll(true),
  m_AutoScrollMargin(10),
  m_autoScrollCount(0),
  m_InputParametersWidget(NULL),
  m_PipelineMessageObserver(NULL),
  m_StatusBar(NULL)
{
  setupGui();
  m_LastDragPoint = QPoint(-1, -1);
  m_autoScrollTimer.setParent(this);

  setContextMenuPolicy(Qt::CustomContextMenu);
  setFocusPolicy(Qt::StrongFocus);

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(requestContextMenu(const QPoint&)));
  connect(this, SIGNAL(contextMenuRequested(PipelineViewWidget*, const QPoint&)), dream3dApp, SLOT(on_pipelineViewWidget_contextMenuRequested(PipelineViewWidget*, const QPoint&)));
  }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewWidget::~PipelineViewWidget()
{
  
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  m_PipelineMessageObserver = pipelineMessageObserver;
  // setup our connection
  connect(this, SIGNAL(pipelineIssuesCleared()),
          m_PipelineMessageObserver, SLOT(clearIssues()) );
  connect(this, SIGNAL(preflightPipelineComplete()),
          m_PipelineMessageObserver, SLOT(displayCachedMessages()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setupGui()
{
  newEmptyPipelineViewLayout();
  connect(&m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(doAutoScroll()));

  connect(this, SIGNAL(filterWidgetsRemoved(QList<PipelineFilterWidget*>)), dream3dApp, SLOT(removeFilterWidgets(QList<PipelineFilterWidget*>)));

  m_DropBox = new DropBoxWidget();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setScrollArea(QScrollArea* sa)
{
  m_ScrollArea = sa;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::newEmptyPipelineViewLayout()
{
  if(m_EmptyPipelineLabel == NULL)
  {
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    QSpacerItem* verticalSpacer = new QSpacerItem(20, 341, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(verticalSpacer, 0, 1, 1, 1);

    QSpacerItem* horizontalSpacer_3 = new QSpacerItem(102, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_3, 1, 0, 1, 1);

    m_EmptyPipelineLabel = new QLabel(this);
    m_EmptyPipelineLabel->setObjectName(QString::fromUtf8("label"));
    m_EmptyPipelineLabel->setMinimumSize(QSize(325, 250));
    m_EmptyPipelineLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
                                                          "border-radius: 20px;\n"
                                                          "/*border: 1px solid rgb(120, 120, 120);*/\n"
                                                          "/* background-color: rgb(160, 160, 160); */\n"
                                                          "font-weight: bold;\n"
                                                          "color : rgb(150, 150, 150);\n"
                                                          "text-align: center;\n"
                                                          "margin: 5px;\n"
                                                          "padding: 10px;\n"
                                                          "}"));
    m_EmptyPipelineLabel->setAlignment(Qt::AlignCenter);
    QString text;
    QTextStream ss (&text);
    ss << "<h2>Creating a Pipeline</h2>";
    ss << "<hr>";
    ss << "File > Open <br />";
    ss << "File > New <br />";
    ss << "Drag and drop filters<br />";
    ss << "Double click a Bookmark<br />";
    ss << "Double click a Prebuilt Pipeline<br />";
    m_EmptyPipelineLabel->setText(text);


    gridLayout->addWidget(m_EmptyPipelineLabel, 1, 1, 1, 1);

    QSpacerItem* horizontalSpacer_4 = new QSpacerItem(102, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_4, 1, 2, 1, 1);

    QSpacerItem* verticalSpacer_2 = new QSpacerItem(20, 341, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(verticalSpacer_2, 2, 1, 1, 1);
  }
  emit pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewWidget::filterCount()
{
  int count = 0;
  if (NULL != m_FilterWidgetLayout)
  {
    count = m_FilterWidgetLayout->count() - 1;
  }
  return count;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineFilterWidget* PipelineViewWidget::filterWidgetAt(int index)
{
  PipelineFilterWidget* fw = NULL;
  if (m_FilterWidgetLayout != NULL)
  {
    QWidget* w = m_FilterWidgetLayout->itemAt(index)->widget();
    fw = qobject_cast<PipelineFilterWidget*>(w);
  }
  return fw;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewWidget::indexOfFilterWidget(PipelineFilterWidget* filterWidget)
{
  return m_FilterWidgetLayout->indexOf(filterWidget);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::resetLayout()
{
  // Check to see if we have removed all the filters
  if (filterCount() <= 0)
  {
    // Emit a signal to tell SIMPLView_UI to erase the Filter Input Widget.
    emit filterInputWidgetNeedsCleared();

    // Remove the current Layout
    QLayout* l = layout();
    if (NULL != l && l == m_FilterWidgetLayout)
    {
      qDeleteAll(l->children());
      delete l;
      m_FilterWidgetLayout = NULL;
    }

    // and add the empty pipeline layout instead
    newEmptyPipelineViewLayout();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::clearWidgets()
{
  emit pipelineIssuesCleared();
  emit pipelineChanged();

  qint32 count = filterCount();
  for(qint32 i = count - 1; i >= 0; --i)
  {
    QWidget* w = m_FilterWidgetLayout->itemAt(i)->widget();
    QSpacerItem* spacer = m_FilterWidgetLayout->itemAt(i)->spacerItem();
    if (NULL != w)
    {
      m_FilterWidgetLayout->removeWidget(w);
      PipelineFilterWidget* fw = qobject_cast<PipelineFilterWidget*>(w);
      if(fw)
      {
        fw->getFilter()->setPreviousFilter(AbstractFilter::NullPointer());
        fw->getFilter()->setNextFilter(AbstractFilter::NullPointer());
      }
      w->deleteLater();
    }
    else if (NULL != spacer)
    {
      m_FilterWidgetLayout->removeItem(spacer);
    }
  }
  resetLayout();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::reindexWidgetTitles()
{
  qint32 count = filterCount();
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      QString hl = fw->getFilter()->getHumanLabel();
      hl = QString("[") + QString::number(i + 1) + QString("] ") + hl;
      fw->setFilterTitle(hl);
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewWidget::getFilterPipeline()
{
  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  qint32 count = filterCount();
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      AbstractFilter::Pointer filter = fw->getFilter();
      pipeline->pushBack(filter);
    }

  }
  pipeline->addMessageReceiver(m_PipelineMessageObserver);
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewWidget::getCopyOfFilterPipeline()
{
  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  qint32 count = filterCount();
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      AbstractFilter::Pointer filter = fw->getFilter();
      AbstractFilter::Pointer copy = filter->newFilterInstance(true);
      pipeline->pushBack(copy);
    }

  }
  pipeline->addMessageReceiver(m_PipelineMessageObserver);
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewWidget::writePipeline(QString filePath)
{
  QFileInfo fi(filePath);
  QString ext = fi.completeSuffix();

  //If the filePath already exists - delete it so that we get a clean write to the file
  if (fi.exists() == true && (ext == "dream3d" || ext == "json"))
  {
    QFile f(filePath);
    if (f.remove() == false)
    {
      QMessageBox::warning(this, QString::fromLatin1("Pipeline Write Error"),
                           QString::fromLatin1("There was an error removing the existing pipeline file. The pipeline was NOT saved."));
      return -1;
    }
  }

  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = getFilterPipeline();

  int err = 0;
  if (ext == "dream3d")
  {
    err = H5FilterParametersWriter::WritePipelineToFile(pipeline, fi.absoluteFilePath(), fi.fileName(), reinterpret_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else if (ext == "json")
  {
    err = JsonFilterParametersWriter::WritePipelineToFile(pipeline, fi.absoluteFilePath(), fi.fileName(), reinterpret_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else
  {
    m_StatusBar->showMessage(tr("The pipeline was not written to file '%1'. '%2' is an unsupported file extension.").arg(fi.fileName()).arg(ext));
    return -1;
  }

  if (err < 0)
  {
    m_StatusBar->showMessage(tr("There was an error while saving the pipeline to file '%1'.").arg(fi.fileName()));
    return -1;
  }
  else
  {
    m_StatusBar->showMessage(tr("The pipeline has been saved successfully to '%1'.").arg(fi.fileName()));
  }

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewWidget::openPipeline(const QString& filePath, int index, const bool& setOpenedFilePath, const bool& changeTitle)
{
  QFileInfo fi(filePath);
  if (fi.exists() == false)
  {
    QMessageBox::warning(this, QString::fromLatin1("Pipeline Read Error"),
                         QString::fromLatin1("There was an error opening the specified pipeline file. The pipeline file does not exist."));
    return -1;
  }

  // Clear the pipeline Issues table first so we can collect all the error messages
  emit pipelineIssuesCleared();

  QString ext = fi.suffix();
  QString name = fi.fileName();

  // Read the pipeline from the file
  FilterPipeline::Pointer pipeline = readPipelineFromFile(filePath);

  // Check that a valid extension was read...
  if (pipeline == FilterPipeline::NullPointer())
  {
    m_StatusBar->showMessage(tr("The pipeline was not read correctly from file '%1'. '%2' is an unsupported file extension.").arg(name).arg(ext));
    return -1;
  }

  // Populate the pipeline view
  populatePipelineView(pipeline, index);

  // Notify user of successful read
  m_StatusBar->showMessage(tr("The pipeline has been read successfully from '%1'.").arg(name));

  QString file = filePath;
  emit pipelineOpened(file, setOpenedFilePath, changeTitle);

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewWidget::readPipelineFromFile(const QString& filePath)
{
  QFileInfo fi(filePath);
  QString ext = fi.suffix();
  QString name = fi.fileName();

  FilterPipeline::Pointer pipeline;
  if (ext == "ini" || ext == "txt")
  {
    pipeline = QFilterParametersReader::ReadPipelineFromFile(filePath, QSettings::IniFormat, dynamic_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else if (ext == "dream3d")
  {
    pipeline = H5FilterParametersReader::ReadPipelineFromFile(filePath);
  }
  else if (ext == "json")
  {
    pipeline = JsonFilterParametersReader::ReadPipelineFromFile(filePath);
  }
  else
  {
    pipeline = FilterPipeline::NullPointer();
  }

  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::addFilter(const QString& filterClassName, int index)
{
  if (this->isEnabled() == false) { return; }
  FilterManager* fm = FilterManager::Instance();
  if(NULL == fm) { return; }
  IFilterFactory::Pointer wf = fm->getFactoryForFilter(filterClassName);
  if (NULL == wf.get()) { return; }

  // Create an instance of the filter. Since we are dealing with the AbstractFilter interface we can not
  // actually use the concrete filter class. We are going to have to rely on QProperties or Signals/Slots
  // to communicate changes back to the filter.
  AbstractFilter::Pointer filter = wf->create();

  if (index < 0) // If the programmer wants to add it to the end of the list
  {
    index = filterCount();
  }

  // Create a FilterWidget object
  PipelineFilterWidget* w = new PipelineFilterWidget(filter, NULL, this);

  // Add the filter widget to this view widget
  addFilterWidget(w, index);

  // Clear the pipeline Issues table first so we can collect all the error messages
  emit pipelineIssuesCleared();
  // Now preflight the pipeline for this filter.
  preflightPipeline();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::addFilterWidget(PipelineFilterWidget* fw, int index)
{
  bool addSpacer = false;
  if (filterCount() <= 0)
  {
    if (NULL != m_EmptyPipelineLabel)
    {
      m_EmptyPipelineLabel->hide();
      delete m_EmptyPipelineLabel;
      m_EmptyPipelineLabel = NULL;
    }
    QLayout* l = layout();
    if (NULL != l)
    {
      qDeleteAll(l->children());
      delete l;
    }

    m_FilterWidgetLayout = new QVBoxLayout(this);
    m_FilterWidgetLayout->setObjectName(QString::fromUtf8("m_FilterWidgetLayout"));
    m_FilterWidgetLayout->setContentsMargins(2, 2, 2, 2);
    m_FilterWidgetLayout->setSpacing(3);
    addSpacer = true;

    if(index < 0)
    {
      index = 0;
    }
  }

  if(index == -1)
  {
    index = filterCount();
  }

  // The layout will take control of the PipelineFilterWidget 'w' instance
  m_FilterWidgetLayout->insertWidget(index, fw);
  // Set the Parent
  fw->setParent(this);

  /// Now setup all the connections between the various widgets

  // When the filter is removed from this view
  connect(fw, SIGNAL(filterWidgetRemoved(PipelineFilterWidget*)),
          dream3dApp, SLOT(removeFilterWidget(PipelineFilterWidget*)) );

  // When the FilterWidget is selected
  connect(fw, SIGNAL(filterWidgetPressed(PipelineFilterWidget*, Qt::KeyboardModifiers)),
          this, SLOT(setSelectedFilterWidget(PipelineFilterWidget*, Qt::KeyboardModifiers)));

  // When the filter widget is dragged
  connect(fw, SIGNAL(dragStarted(QMouseEvent*)),
          this, SLOT(startDrag(QMouseEvent*)));

  connect(fw, SIGNAL(parametersChanged()),
          this, SLOT(preflightPipeline()));

  connect(fw, SIGNAL(parametersChanged()),
          this, SLOT(handleFilterParameterChanged()));

  // Check to make sure at least the vertical spacer is in the Layout
  if (addSpacer)
  {
    QSpacerItem* verticalSpacer = new QSpacerItem(20, 361, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_FilterWidgetLayout->insertSpacerItem(-1, verticalSpacer);
  }

  // Make sure the widget titles are all correct
  reindexWidgetTitles();
  
  // Finally, set this new filter widget as selected
  setSelectedFilterWidget(fw, Qt::NoModifier);

  // Get the filter to ignore Scroll Wheel Events
  fw->installEventFilter( this);

  // Emit that the pipeline changed
  emit pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::startDrag(QMouseEvent* event)
{
  QList<PipelineFilterWidget*> selectedWidgets = getSelectedFilterWidgets();
  m_DraggedFilterWidgets = selectedWidgets;

  QPixmap pixmap = m_ShiftStart->grab();

  int pWidth = pixmap.size().width();
  int pHeight = pixmap.size().height() * selectedWidgets.size() + (3 * (selectedWidgets.size() - 1));

  // Create new picture for transparent
  QPixmap transparent(pWidth, pHeight);
  // Do transparency
  transparent.fill(Qt::transparent);

  QPainter p;
  p.begin(&transparent);
  p.setOpacity(0.70);
  int offset = 0;
  for (int i = 0; i < selectedWidgets.size(); i++)
  {
    QPixmap currentPixmap = selectedWidgets[i]->grab();
    p.drawPixmap(0, offset, currentPixmap);
    offset = offset + pixmap.size().height() + 3;
  }
  p.end();

  PipelineViewPtrMimeData* mimeData = new PipelineViewPtrMimeData();
  mimeData->setPipelineViewPtr(this);

  QDrag* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->setPixmap(transparent);
  drag->setHotSpot(event->pos());

  drag->exec(Qt::CopyAction);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewWidget::eventFilter(QObject* o, QEvent* e)
{
  if ( e->type() == QEvent::Wheel && qobject_cast<PipelineFilterWidget*>(o) )
  {
    return false;
  }
  return QFrame::eventFilter( o, e );
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::preflightPipeline()
{
  emit pipelineIssuesCleared();
  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = getFilterPipeline();

  FilterPipeline::FilterContainerType filters = pipeline->getFilterContainer();
  for(int i = 0; i < filters.size(); i++)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      fw->setHasPreflightErrors(false);
    }
    filters.at(i)->setErrorCondition(0);
  }


  QProgressDialog progress("Preflight Pipeline", "", 0, 1, this);
  progress.setWindowModality(Qt::WindowModal);

  // Preflight the pipeline
  int err = pipeline->preflightPipeline();
  if (err < 0)
  {
    //FIXME: Implement this
  }
  progress.setValue(1);

  int count = pipeline->getFilterContainer().size();
  //Now that the preflight has been executed loop through the filters and check their error condition and set the
  // outline on the filter widget if there were errors or warnings
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      AbstractFilter::Pointer filter = fw->getFilter();
      if(filter->getErrorCondition() < 0) {fw->setHasPreflightErrors(true);}
    }
  }
  emit preflightPipelineComplete();
  emit preflightFinished(err);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::removeFilterWidget(PipelineFilterWidget* whoSent)
{
  if (whoSent)
  {
    QWidget* w = qobject_cast<QWidget*>(whoSent);
    m_FilterWidgetLayout->removeWidget(w);

    if (w)
    {
      whoSent->getFilter()->setPreviousFilter(AbstractFilter::NullPointer());
      whoSent->getFilter()->setNextFilter(AbstractFilter::NullPointer());

      w->deleteLater();
    }
  }

  if (getSelectedFilterWidgets().isEmpty())
  {
    m_ShiftStart = NULL;
  }

  QSpacerItem* spacer = m_FilterWidgetLayout->itemAt(0)->spacerItem();
  if (NULL != spacer)
  {
    m_FilterWidgetLayout->removeItem(spacer);
  }

  reindexWidgetTitles();
  preflightPipeline();

  resetLayout();
  emit pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setSelectedFilterWidget(PipelineFilterWidget* w, Qt::KeyboardModifiers modifiers)
{
  if (modifiers == Qt::ShiftModifier)
  {
    clearSelectedFilterWidgets();

    if (NULL == m_ShiftStart)
    {
      m_ShiftStart = w;
    }

    int begin;
    int end;
    if (m_FilterWidgetLayout->indexOf(w) < m_FilterWidgetLayout->indexOf(m_ShiftStart))
    {
      // The filter widget that was just selected is before the "active" widget
      begin = m_FilterWidgetLayout->indexOf(w);
      end = m_FilterWidgetLayout->indexOf(m_ShiftStart);
    }
    else
    {
      // The filter widget that was just selected is after the "active" widget
      begin = m_FilterWidgetLayout->indexOf(m_ShiftStart);
      end = m_FilterWidgetLayout->indexOf(w);
    }

    for (int i = begin; i <= end; i++)
    {
      filterWidgetAt(i)->setIsSelected(true, modifiers);
    }
  }
  else if (modifiers == Qt::ControlModifier)
  {
    m_ShiftStart = w;

    if (w->isSelected())
    {
      w->setIsSelected(false, modifiers);
      if (getSelectedFilterWidgets().isEmpty())
      {
        m_ShiftStart = NULL;
      }
    }
    else
    {
      w->setIsSelected(true, modifiers);
    }
  }
  else
  {
    clearSelectedFilterWidgets();

    m_ShiftStart = w;
    w->setIsSelected(true, modifiers);
  }

  QList<PipelineFilterWidget*> selectedWidgets = getSelectedFilterWidgets();

  if (selectedWidgets.size() == 1)
  {
    emit filterInputWidgetChanged(selectedWidgets[0]->getFilterInputWidget());
  }
  else
  {
    emit filterInputWidgetNeedsCleared();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::clearSelectedFilterWidgets()
{
  for (int i=0; i<filterCount(); i++)
  {
    if (NULL != dynamic_cast<DropBoxWidget*>(m_FilterWidgetLayout->itemAt(i)->widget()))
    {
      continue;
    }
    else
    {
      PipelineFilterWidget* fw = dynamic_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
      fw->setIsSelected(false);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setStatusBar(QStatusBar* statusBar)
{
  m_StatusBar = statusBar;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::addSIMPLViewReaderFilter(const QString& filePath, int index)
{
  DataContainerReader::Pointer reader = DataContainerReader::New();
  reader->setInputFile(filePath);

  // Create a PipelineFilterWidget using the current AbstractFilter instance to initialize it
  PipelineFilterWidget* w = new PipelineFilterWidget(reader, NULL, this);
  addFilterWidget(w, index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::populatePipelineView(FilterPipeline::Pointer pipeline, int index)
{
  if (NULL == pipeline.get()) { clearWidgets(); return; }

  QString jsonString = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Pipeline");

  emit filterWidgetsPasted(jsonString, index);

  if (filterCount() > 0)
  {
    PipelineFilterWidget* fw = dynamic_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(0)->widget());
    if (fw)
    {
      setSelectedFilterWidget(fw, Qt::NoModifier);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
  {
    QList<PipelineFilterWidget*> selectedWidgets = getSelectedFilterWidgets();

    if (selectedWidgets.size() > 0)
    {
      emit filterWidgetsRemoved(selectedWidgets);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewWidget::containsFilterWidget(PipelineFilterWidget* filterWidget)
{
  for (int i = 0; i < m_FilterWidgetLayout->count(); i++)
  {
    if (dynamic_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget()) == filterWidget)
    {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dragMoveEvent(QDragMoveEvent* event)
{
  m_LastDragPoint = event->pos();

  // Remove the drop box, if it exists
  if (NULL != m_FilterWidgetLayout && m_FilterWidgetLayout->indexOf(m_DropBox) != -1)
  {
    m_FilterWidgetLayout->removeWidget(m_DropBox);
    m_DropBox->setParent(NULL);
  }

  // If cursor is within margin boundaries, start scrolling
  if (shouldAutoScroll(event->pos()))
  {
    startAutoScroll();
  }
  // Otherwise, stop scrolling
  else
  {
    stopAutoScroll();
  }

  const QMimeData* mimedata = event->mimeData();
  if (NULL != dynamic_cast<const PipelineViewPtrMimeData*>(mimedata))
  {
    // The user is moving existing filter widgets, either within the same pipeline view or between pipeline views
    PipelineViewWidget* origin = dynamic_cast<const PipelineViewPtrMimeData*>(mimedata)->getPipelineViewPtr();

    if (origin == this)
    {
      if (qApp->queryKeyboardModifiers() != Qt::AltModifier)
      {
        if (m_DraggedFilterWidgets.size() > 1)
        {
          event->ignore();
          return;
        }
        else if (m_DraggedFilterWidgets.size() == 1)
        {
          // Remove the filter widget
          if (NULL != m_FilterWidgetLayout && origin->containsFilterWidget(m_DraggedFilterWidgets[0]))
          {
            PipelineFilterWidget* draggedWidget = m_DraggedFilterWidgets[0];
            m_FilterOrigPos = origin->indexOfFilterWidget(draggedWidget);
            m_FilterWidgetLayout->removeWidget(draggedWidget);
            draggedWidget->setParent(NULL);
          }
        }
      }
    }

    int count = filterCount();
    for (int i = 0; i <= count; ++i)
    {
      PipelineFilterWidget* w;
      if (NULL == m_FilterWidgetLayout)
      {
        return;
      }
      else if (i == count)
      {
        w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i-1)->widget());
      }
      else
      {
        w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
      }
      if (w != NULL)
      {
        if ((i == count && event->pos().y() >= w->geometry().y() + w->geometry().height() / 2) || (event->pos().y() <= w->geometry().y() + w->geometry().height() / 2))
        {
          if (m_DraggedFilterWidgets.size() > 1)
          {
            m_DropBox->setLabel("Place " + QString::number(m_DraggedFilterWidgets.size()) + " Filters Here");
          }
          else if (m_DraggedFilterWidgets.size() == 1)
          {
            m_DropBox->setLabel("    [" + QString::number(i + 1) + "] " + m_DraggedFilterWidgets[0]->getHumanLabel());
          }
          else
          {
            event->ignore();
            return;
          }
          m_FilterWidgetLayout->insertWidget(i, m_DropBox);
          reindexWidgetTitles();
          break;
        }
      }
    }
  }
  else if (mimedata->hasUrls() || mimedata->hasFormat(SIMPL::DragAndDrop::BookmarkItem) || mimedata->hasFormat(SIMPL::DragAndDrop::FilterItem))
  {
    QString data;
    if (mimedata->hasUrls())
    {
      data = mimedata->text();
      QUrl url(data);
      data = url.toLocalFile();
    }
    else if (mimedata->hasText())
    {
      data = mimedata->text();
    }
    else if (mimedata->hasFormat(SIMPL::DragAndDrop::BookmarkItem))
    {
      QByteArray jsonArray = mimedata->data(SIMPL::DragAndDrop::BookmarkItem);
      QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
      QJsonObject obj = doc.object();

      if (obj.size() > 1)
      {
        event->accept();
        return;
      }

      QJsonObject::iterator iter = obj.begin();
      data = iter.value().toString();
    }
    else
    {
      QByteArray jsonArray = mimedata->data(SIMPL::DragAndDrop::FilterItem);
      QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
      QJsonObject obj = doc.object();
      QJsonObject::iterator iter = obj.begin();
      data = iter.value().toString();
    }

    QFileInfo fi(data);
    QString ext = fi.completeSuffix();
    FilterManager* fm = FilterManager::Instance();
    if (NULL == fm) { return; }
    IFilterFactory::Pointer wf = fm->getFactoryForFilter(data);

    // If the dragged item is a filter item...
    if (NULL != wf)
    {
      QString humanName = wf->getFilterHumanLabel();

      bool didInsert = false;
      // This path is taken if a filter is dragged from the list of filters

      int count = filterCount();
      for (int i = 0; i < count; ++i)
      {
        PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
        if (NULL != w && event->pos().y() <= w->geometry().y() + w->geometry().height() / 2)
        {
          m_DropBox->setLabel("    [" + QString::number(i + 1) + "] " + humanName);
          m_FilterWidgetLayout->insertWidget(i, m_DropBox);
          reindexWidgetTitles();
          didInsert = true;
          break;
        }
      }
      // Check to see if we are trying to move it to the end
      if (false == didInsert && count > 0)
      {
        PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(count - 1)->widget());
        if (NULL != w && event->pos().y() >= w->geometry().y() + w->geometry().height() / 2)
        {
          m_DropBox->setLabel("    [" + QString::number(count) + "] " + humanName);
          m_FilterWidgetLayout->insertWidget(count, m_DropBox);
          reindexWidgetTitles();
        }
      }

      event->accept();
    }
    // If the dragged item is a pipeline file...
    else if (ext == "dream3d" || ext == "json" || ext == "ini" || ext == "txt")
    {
      QString pipelineName = fi.baseName();

      bool didInsert = false;
      // This path is taken if a filter is dragged from the list of filters

      int count = filterCount();
      for (int i = 0; i < count; ++i)
      {
        PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
        if (NULL != w && event->pos().y() <= w->geometry().y() + w->geometry().height() / 2)
        {
          m_DropBox->setLabel("Place '" + pipelineName + "' Here");
          m_FilterWidgetLayout->insertWidget(i, m_DropBox);
          reindexWidgetTitles();
          didInsert = true;
          break;
        }
      }
      // Check to see if we are trying to move it to the end
      if (false == didInsert && count > 0)
      {
        PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(count - 1)->widget());
        if (NULL != w && event->pos().y() >= w->geometry().y() + w->geometry().height() / 2)
        {
          m_DropBox->setLabel("Place '" + pipelineName + "' Here");
          m_FilterWidgetLayout->insertWidget(count, m_DropBox);
          reindexWidgetTitles();
        }
      }

      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dropEvent(QDropEvent* event)
{
  const QMimeData* mimedata = event->mimeData();
  if (mimedata->hasUrls() || mimedata->hasText() || mimedata->hasFormat(SIMPL::DragAndDrop::BookmarkItem) || mimedata->hasFormat(SIMPL::DragAndDrop::FilterItem))
  {
    QString data;

    if (mimedata->hasUrls())
    {
      data = mimedata->text();
      QUrl url(data);
      data = url.toLocalFile();
    }
    else if (mimedata->hasText())
    {
      data = mimedata->text();
    }
    else if (mimedata->hasFormat(SIMPL::DragAndDrop::FilterItem))
    {
      QByteArray jsonArray = mimedata->data(SIMPL::DragAndDrop::FilterItem);
      QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
      QJsonObject obj = doc.object();
      QJsonObject::iterator iter = obj.begin();
      data = iter.value().toString();
    }
    else
    {
      QByteArray jsonArray = mimedata->data(SIMPL::DragAndDrop::BookmarkItem);
      QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
      QJsonObject obj = doc.object();

      if (obj.size() > 1)
      {
        QMessageBox::warning(NULL, "SIMPLView Warning", "SIMPLView currently does not support dragging and dropping multiple bookmarks.", QMessageBox::Ok);
        event->ignore();
        return;
      }

      QJsonObject::iterator iter = obj.begin();
      data = iter.value().toString();
    }

    QFileInfo fi(data);
    QString ext = fi.completeSuffix();
    FilterManager* fm = FilterManager::Instance();
    if (NULL == fm) { return; }
    IFilterFactory::Pointer wf = fm->getFactoryForFilter(data);

    // If the dragged item is a filter item...
    if (NULL != wf)
    {
      int index;

      // We need to figure out where it was dropped relative to other filters
      if (NULL != m_FilterWidgetLayout && m_FilterWidgetLayout->indexOf(m_DropBox) != -1)
      {
        index = m_FilterWidgetLayout->indexOf(m_DropBox);
      }
      else
      {
        index = -1;
      }

      // Now that we have an index, insert the filter.
      addFilter(data, index);

      emit pipelineChanged();
      event->accept();
    }
    // If the dragged item is a pipeline file...
    else if (ext == "dream3d" || ext == "json" || ext == "ini" || ext == "txt")
    {
      int index = 0;
      if (NULL != m_FilterWidgetLayout)
      {
        index = m_FilterWidgetLayout->indexOf(m_DropBox);
      }

      if (ext == "json" || ext == "ini" || ext == "txt")
      {
        openPipeline(data, index, false, false);

        emit pipelineChanged();
      }
      else if (ext == "dream3d")
      {
        FileDragMessageBox* msgBox = new FileDragMessageBox(this);
        msgBox->exec();
        msgBox->deleteLater();

        if (msgBox->didPressOkBtn() == true)
        {
          if (msgBox->isExtractPipelineBtnChecked() == true)
          {
            openPipeline(data, index, false, false);
          }
          else
          {
            addSIMPLViewReaderFilter(data, index);
            emit pipelineChanged();
          }
        }
      }
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
  else if (NULL != dynamic_cast<const PipelineViewPtrMimeData*>(mimedata))
  {
    PipelineViewWidget* origin = dynamic_cast<const PipelineViewPtrMimeData*>(mimedata)->getPipelineViewPtr();

    // The drop box, if it exists, marks the index where the filter should be dropped
    int index;
    if (NULL != m_FilterWidgetLayout && m_FilterWidgetLayout->indexOf(m_DropBox) != -1)
    {
      index = m_FilterWidgetLayout->indexOf(m_DropBox);
      m_FilterWidgetLayout->removeWidget(m_DropBox);
      m_DropBox->setParent(NULL);
    }
    else
    {
      index = filterCount();
    }

    if (origin != this || (origin == this && qApp->queryKeyboardModifiers() == Qt::AltModifier))
    {
      emit filterWidgetsDropped(index, qApp->queryKeyboardModifiers());
      origin->m_DraggedFilterWidgets.clear();
      event->accept();
    }
    else if (m_DraggedFilterWidgets.size() == 1)
    {
      emit moveCommandNeeded(m_DraggedFilterWidgets[0], m_FilterOrigPos, index, this);
      m_DraggedFilterWidgets.clear();
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }

  // Stop auto scrolling if widget is dropped
  stopAutoScroll();

  // Remove the drop line, if it exists
  if (NULL != m_FilterWidgetLayout && m_FilterWidgetLayout->indexOf(m_DropBox) != -1)
  {
    m_FilterWidgetLayout->removeWidget(m_DropBox);
    m_DropBox->setParent(NULL);
    reindexWidgetTitles();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
  int index;
  if (NULL != m_FilterWidgetLayout)
  {
    index = m_FilterWidgetLayout->indexOf(m_DropBox);
  }

  // Remove the placeholder drop box
  if (NULL != m_FilterWidgetLayout && index != -1)
  {
    m_FilterWidgetLayout->removeWidget(m_DropBox);
    m_DropBox->setParent(NULL);
  }

  // Put filter widget back to original position
  if (m_DraggedFilterWidgets.size() == 1 && qApp->queryKeyboardModifiers() != Qt::AltModifier)
  {
    m_FilterWidgetLayout->insertWidget(m_FilterOrigPos, m_DraggedFilterWidgets[0]);
    setSelectedFilterWidget(m_DraggedFilterWidgets[0], Qt::NoModifier);
  }

  reindexWidgetTitles();

//  // Set the current filter as previous, and nullify the current
//  m_PreviousFilterBeingDragged = m_CurrentFilterBeingDragged;
//  m_CurrentFilterBeingDragged = NULL;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dragEnterEvent(QDragEnterEvent* event)
{
  event->acceptProposedAction();

//  // If there is a previous filter, set it as current
//  if (NULL != m_PreviousFilterBeingDragged && event->dropAction() == Qt::MoveAction)
//  {
//    m_CurrentFilterBeingDragged = m_PreviousFilterBeingDragged;
//  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::stopAutoScroll()
{
  m_autoScrollTimer.stop();
  m_autoScrollCount = 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::startAutoScroll()
{
  int scrollInterval = 50;
  m_autoScrollTimer.start(scrollInterval);
  m_autoScrollCount = 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::doAutoScroll()
{
  // find how much we should scroll with
  int verticalStep = m_ScrollArea->verticalScrollBar()->pageStep();
  int horizontalStep = m_ScrollArea->horizontalScrollBar()->pageStep();
  if (m_autoScrollCount < qMax(verticalStep, horizontalStep))
  { m_autoScrollCount = m_autoScrollCount + 15; }

  int margin = m_AutoScrollMargin;
  int verticalValue = m_ScrollArea->verticalScrollBar()->value();
  int horizontalValue = m_ScrollArea->horizontalScrollBar()->value();

  QPoint pos = m_ScrollArea->viewport()->mapFromGlobal(QCursor::pos());
  QRect area = m_ScrollArea->geometry();

  // do the scrolling if we are in the scroll margins
  if (pos.y() - area.top() < margin)
  { m_ScrollArea->verticalScrollBar()->setValue(verticalValue - m_autoScrollCount); }
  else if (area.bottom() - pos.y() < margin)
  { m_ScrollArea-> verticalScrollBar()->setValue(verticalValue + m_autoScrollCount); }
  //  if (pos.x() - area.left() < margin)
  //    m_ScrollArea->horizontalScrollBar()->setValue(horizontalValue - d->m_autoScrollCount);
  //  else if (area.right() - pos.x() < margin)
  //    m_ScrollArea->horizontalScrollBar()->setValue(horizontalValue + d->m_autoScrollCount);
  // if nothing changed, stop scrolling
  bool verticalUnchanged = (verticalValue == m_ScrollArea->verticalScrollBar()->value());
  bool horizontalUnchanged = (horizontalValue == m_ScrollArea->horizontalScrollBar()->value());
  if (verticalUnchanged && horizontalUnchanged)
  {
    stopAutoScroll();
  }
  else
  {
    m_ScrollArea->viewport()->update();
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewWidget::shouldAutoScroll(const QPoint& pos)
{
  if (NULL == m_ScrollArea) { return false; }
  QPoint scpos = m_ScrollArea->viewport()->mapFromGlobal(QCursor::pos());
  QRect rect = m_ScrollArea->geometry();

  if (scpos.y() <= getAutoScrollMargin())
  {
    return true;
  }
  else if (pos.y() >= rect.height() - getAutoScrollMargin())
  {
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    clearSelectedFilterWidgets();
    m_ShiftStart = NULL;
    emit filterInputWidgetNeedsCleared();
  }

  QFrame::mousePressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::requestContextMenu(const QPoint& pos)
{
  activateWindow();
  emit contextMenuRequested(this, pos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setContextMenuActions(QList<QAction*> list)
{
  m_MenuActions = list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStatusBar* PipelineViewWidget::getStatusBar()
{
  return m_StatusBar;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::handleFilterParameterChanged()
{
  emit filterInputWidgetEdited();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::toRunningState()
{
  setAcceptDrops(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::toIdleState()
{
  setAcceptDrops(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::showFilterHelp(const QString& className)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<PipelineFilterWidget*> PipelineViewWidget::getSelectedFilterWidgets()
{
  QList<PipelineFilterWidget*> filterWidgets;
  for (int i=0; i<filterCount(); i++)
  {
    if (NULL != dynamic_cast<DropBoxWidget*>(m_FilterWidgetLayout->itemAt(i)->widget()))
    {
      continue;
    }
    else
    {
      PipelineFilterWidget* fw = dynamic_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
      if (fw->isSelected() == true)
      {
        filterWidgets.push_back(fw);
      }
    }
  }
  return filterWidgets;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<PipelineFilterWidget*> PipelineViewWidget::getDraggedFilterWidgets()
{
  return m_DraggedFilterWidgets;
}

