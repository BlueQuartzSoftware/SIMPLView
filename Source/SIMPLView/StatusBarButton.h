#ifndef _statusbarbutton_H_
#define _statusbarbutton_H_

#include <QtCore/QObject>
#include <QtWidgets/QToolButton>

class StatusBarButton : public QToolButton
{
    Q_OBJECT
  public:
    StatusBarButton(QWidget* parent = nullptr);
    
    virtual ~StatusBarButton();
    
    /**
   * @brief paintEvent
   * @param event
   */
    void paintEvent(QPaintEvent* event) override;
    
    void setFlat(bool b){};
    
  public slots:
    
    void setBadgeCount(int count);
    
  protected:
    
    /**
     * @brief mousePressEvent
     * @param event
     */
    virtual void mousePressEvent( QMouseEvent* event ) override;
    
    /**
     * @brief mouseReleaseEvent
     * @param event
     */
    virtual void mouseReleaseEvent( QMouseEvent* event ) override;
    
    
  private:
    int m_BadgeCount = 0;
    int m_TextMargin = 6;
    
    qreal m_BorderThickness = 0.0;
    qreal m_BorderIncrement = 1.0;
    
    int m_BadgeDiameter = 20;
    bool m_Pressed = false;

    /**
   * @brief initialize Calls all the necessary initialization code for the widget
   * @param filter
   */
    void initialize();
    
    StatusBarButton(const StatusBarButton&) = delete; // Copy Constructor Not Implemented
    void operator=(const StatusBarButton&) = delete;  // Operator '=' Not Implemented
};

#endif /* _statusbarbutton_H_ */
