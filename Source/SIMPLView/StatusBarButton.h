#ifndef _statusbarbutton_H_
#define _statusbarbutton_H_

#include <QtCore/QObject>
#include <QtWidgets/QPushButton>

class StatusBarButton : public QPushButton
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

public slots:

  void setBadgeCount(int count);

protected:
private:
  int m_BadgeCount = 0;
  bool m_HoverState = false;
  int m_TextMargin = 6;

  qreal m_BorderThickness = 0.0;
  qreal m_BorderIncrement = 1.0;

  int m_BadgeDiameter = 20;

  /**
   * @brief initialize Calls all the necessary initialization code for the widget
   * @param filter
   */
  void initialize();

  StatusBarButton(const StatusBarButton&) = delete; // Copy Constructor Not Implemented
  void operator=(const StatusBarButton&) = delete;  // Operator '=' Not Implemented
};

#endif /* _statusbarbutton_H_ */
