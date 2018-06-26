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



#ifndef _simplview_ui_h_
#define _simplview_ui_h_


//-- Qt Includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QToolBar>

#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Plugin/ISIMPLibPlugin.h"
#include "SIMPLib/Filtering/FilterPipeline.h"

#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/QtSupport/QtSSettings.h"

//-- UIC generated Header
#include "ui_SIMPLView_UI.h"


class ISIMPLibPlugin;
class FilterLibraryToolboxWidget;
class BookmarksToolboxWidget;
class FilterListWidget;
class UpdateCheckDialog;
class UpdateCheckData;
class UpdateCheck;
class QToolButton;
class AboutSIMPLView;
class StatusBarWidget;
class PipelineTreeView;
class PipelineModel;
class PipelineListWidget;
class SVPipelineViewWidget;
class SIMPLViewMenuItems;

/**
* @class SIMPLView_UI SIMPLView_UI Applications/SIMPLView/SIMPLView_UI.h
* @brief The class is the actual implementation for the GUI window that appears on screen
* when the application is launched. It contains all the subwidgets that the user
* can utilize to perform the various actions of the SIMPLView Software Suite.
* @author Michael A. Jackson for BlueQuartz Software
* @date Oct 19, 2009
* @version 1.0
*/
class SIMPLView_UI : public QMainWindow
{
    Q_OBJECT

  public:
    enum class HideDockSetting : int
    {
      Ignore = 0,
      OnError = 1,
      OnStatusAndError = 2
    };

    SIMPLView_UI(QWidget* parent = nullptr);
    virtual ~SIMPLView_UI();

    /**
     * @brief setLoadedPlugins This will set the plugins that have already been loaded by another mechanism. The plugins are NOT
     * deleted by this class and the unloading and clean up of the plugin pointers is the responsibility of the caller.
     * @param plugins The plugins that adhere to the ISIMPLibPlugin
     */
    void setLoadedPlugins(QVector<ISIMPLibPlugin*> plugins);

    /**
     * @brief getDataStructureWidget
     * @return
     */
    DataStructureWidget* getDataStructureWidget();

    /**
    * @brief Reads the preferences from the users pref file
    */
    void readSettings();

    /**
     * @brief Writes the preferences to the users pref file
     */
    void writeSettings();

    /**
     * @brief openPipeline
     * @param filePath
     * @return
     */
    int openPipeline(const QString& filePath);

    /**
     * @brief executePipeline
     */
    void executePipeline();

  public slots:
    /**
    * @brief setFilterBeingDragged
    * @param msg
    */
    void setStatusBarMessage(const QString& msg);

    /**
    * @brief addStdOutputMessage
    * @param msg
    */
    void addStdOutputMessage(const QString& msg);

    /**
     * @brief showFilterHelp
     * @param className
     */
    void showFilterHelp(const QString& className);

    /**
     * @brief showFilterHelp
     * @param url
     */
    void showFilterHelpUrl(const QUrl& url);

    /**
    * @brief clearFilterInputWidget
    */
    void clearFilterInputWidget();

    /**
     * @brief listenSavePipelineTriggered
     */
    void listenSavePipelineTriggered();

    /**
     * @brief listenSavePipelineAsTriggered
     */
    void listenSavePipelineAsTriggered();

  protected:

    /**
     * @brief populateMenus This is a planned API that plugins would use to add Menus to the main application
     * @param plugin
     */
    void populateMenus(QObject* plugin);

    /**
    * @brief
    */
    void connectSignalsSlots();

    /**
     * @brief Implements the CloseEvent to Quit the application and write settings
     * to the preference file
     */
    void closeEvent(QCloseEvent* event);

    /**
     * @brief changeEvent
     * @param event
     */
    void changeEvent(QEvent* event);

    /**
     *
     * @param prefs
     */
    void writeWindowSettings(QtSSettings* prefs);
    void writeVersionCheckSettings(QtSSettings* prefs);

    void readWindowSettings(QtSSettings* prefs);
    void readVersionSettings(QtSSettings* prefs);

    /**
     * @brief Initializes some of the GUI elements with selections or other GUI related items
     */
    void setupGui();

    /**
     * @brief SIMPLView_UI::setupDockWidget
     * @param prefs
     * @param dw
     */
    void readDockWidgetSettings(QtSSettings* prefs, QDockWidget* dw);

    /**
    * @brief SIMPLView_UI::setupDockWidget
    * @param prefs
    * @param value
    */
    void readHideDockSettings(QtSSettings* prefs, HideDockSetting& value);

    /**
     * @brief writeDockWidgetSettings
     * @param prefs
     * @param dw
     */
    void writeDockWidgetSettings(QtSSettings* prefs, QDockWidget* dw);

    /**
    * @brief writeHideDockSettings
    * @param prefs
    * @param value
    */
    void writeHideDockSettings(QtSSettings* prefs, HideDockSetting value);

    /**
     * @brief Checks the currently open file for changes that need to be saved
     * @return QMessageBox::StandardButton
     */
    QMessageBox::StandardButton checkDirtyDocument();

    /**
     * @brief Over ride the resize event
     * @param event The event to process
     */
    void resizeEvent ( QResizeEvent* event );

    /**
    * @brief activateBookmark
    * @param filePath
    * @param execute
    */
    void activateBookmark(const QString& filePath, bool execute);

    /**
    * @brief handlePipelineChanges
    */
    void handlePipelineChanges();

  protected slots:
    /**
     * @brief pipelineDidFinish
     */
    void pipelineDidFinish();

    /**
     * @brief processPipelineMessage
     * @param msg
     */
    void processPipelineMessage(const PipelineMessage& msg);

    /**
    * @brief setFilterInputWidget
    * @param widget
    */
    void setFilterInputWidget(FilterInputWidget* widget);

    /**
    * @brief markDocumentAsDirty
    */
    void markDocumentAsDirty();

    /**
    * @brief issuesTableHasErrors
    * @param hasErrors
    */
    void issuesTableHasErrors(bool hasErrors, int errCount, int warnCount);

    /**
    * @brief Update the FilterInputWidget based on the updated selection
    * @param selected
    * @param deselected
    */
    void filterSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    // Our Signals that we can emit custom for this class
  signals:
    void parentResized();

    /**
    * @brief dream3dWindowChangedState
    */
    void dream3dWindowChangedState(SIMPLView_UI* self);

  private:
    QSharedPointer<Ui::SIMPLView_UI>        m_Ui;
    QMenuBar*                               m_SIMPLViewMenu = nullptr;

    QVector<ISIMPLibPlugin*>                m_LoadedPlugins;

    FilterManager*                          m_FilterManager = nullptr;
    FilterWidgetManager*                    m_FilterWidgetManager = nullptr;

//    StatusBarWidget*                        m_StatusBar = nullptr;

    QString                                 m_LastOpenedFilePath;

    HideDockSetting                         m_HideErrorTable = HideDockSetting::Ignore;
    HideDockSetting                         m_HideStdOutput = HideDockSetting::Ignore;

    FilterInputWidget*                      m_FilterInputWidget = nullptr;

    QMenu*                                  m_MenuFile = nullptr;
    QMenu*                                  m_MenuEdit = nullptr;
    QMenu*                                  m_MenuView = nullptr;
    QMenu*                                  m_MenuBookmarks = nullptr;
    QMenu*                                  m_MenuPipeline = nullptr;
    QMenu*                                  m_MenuHelp = nullptr;
    QMenu*                                  m_MenuAdvanced = nullptr;
    QMenu*                                  m_MenuThemes = nullptr;

    QAction*                                m_ActionNew = nullptr;
    QAction*                                m_ActionOpen = nullptr;
    QAction*                                m_ActionSave = nullptr;
    QAction*                                m_ActionSaveAs = nullptr;
    QAction*                                m_ActionLoadTheme = nullptr;
    QAction*                                m_ActionSaveTheme = nullptr;
    QAction*                                m_ActionClearRecentFiles = nullptr;
    QAction*                                m_ActionExit = nullptr;
    QAction*                                m_ActionShowSIMPLViewHelp = nullptr;
    QAction*                                m_ActionAboutSIMPLView = nullptr;
    QAction*                                m_ActionCheckForUpdates = nullptr;
    QAction*                                m_ActionPluginInformation = nullptr;
    QAction*                                m_ActionClearCache = nullptr;

    QActionGroup*                           m_ThemeActionGroup = nullptr;

    /**
     * @brief createSIMPLViewMenu
     */
    void createSIMPLViewMenuSystem();

    /**
     * @brief savePipeline
     * @return
     */
    bool savePipeline();

    /**
     * @brief saveAsPipeline
     * @return
     */
    bool savePipelineAs();

    /**
     * @brief getPipelineModel
     * @return
     */
    PipelineModel* getPipelineModel();

    SIMPLView_UI(const SIMPLView_UI&);    // Copy Constructor Not Implemented
    void operator=(const SIMPLView_UI&);  // Move assignment Not Implemented
};

#endif /* _SIMPLView_UI_H_ */

