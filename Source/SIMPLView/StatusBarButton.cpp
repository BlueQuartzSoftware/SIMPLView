#include "StatusBarButton.h"


#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QStyleOption>

#include "SVWidgetsLib/QtSupport/QtSStyles.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatusBarButton::StatusBarButton(QWidget* parent)
: QToolButton (parent)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatusBarButton::~StatusBarButton() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatusBarButton::setBadgeCount(int count)
{
  m_BadgeCount = count;
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatusBarButton::mousePressEvent(QMouseEvent* event)
{
  QToolButton::mousePressEvent(event);
  if(event->button() == Qt::LeftButton)
  {
    m_Pressed = true;
  }
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatusBarButton::mouseReleaseEvent(QMouseEvent* event)
{
  QToolButton::mouseReleaseEvent(event);

  m_Pressed = false;
  
    update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatusBarButton::paintEvent(QPaintEvent* event)
{

 // QToolButton::paintEvent( event);


  //qDebug() << "m_Pressed: " << m_Pressed;
  QStyleOption opt;
  opt.init(this);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  QFont font = QtSStyles::GetHumanLabelFont();

#if defined(Q_OS_MAC)
  font.setPointSize(font.pointSize() - 4);
#elif defined(Q_OS_WIN)
  font.setPointSize(font.pointSize() - 3);
#else
  font.setPointSize(font.pointSize() - 1);
#endif

  int maxHeight = maximumHeight();
  QFontMetrics fontMetrics(font);
  int fontHeight = fontMetrics.height();
  int fontMargin = ((maxHeight - fontHeight) / 2) - 1;

  int indexFontWidth = fontMetrics.width(text());

  painter.setFont(font);

  QColor backgroundColor(200, 200, 200);
  QColor borderColor(120, 120, 120);
  QColor fontColor(50, 50, 50);
  QColor badgeBgColor(225, 25, 25); // Mostly Red?
  QColor badgeFontColor(240, 240, 240);
  qreal borderRadius = 5.0;

  if(isChecked())
  {
    backgroundColor = QColor(120, 120, 120);
    borderColor = QColor(200, 200, 200);
    fontColor = QColor(240, 240, 240);
  }
  
  if(m_Pressed && isChecked())
  {
    backgroundColor = QColor(200, 200, 200);
    fontColor = QColor(50, 50, 50);
    borderColor = QColor(120, 120, 120);

  }
  if(m_Pressed && !isChecked())
  {
    backgroundColor = QColor(120, 120, 120);
    fontColor = QColor(240, 240, 240);
    borderColor = QColor(200, 200, 200);
  }
  QRect rect = this->rect();
#if 1
  QPainterPath buttonPath;
  buttonPath.addRoundedRect(rect, borderRadius, borderRadius);
  QPen pen(borderColor, 2);
  painter.setPen(pen);
  painter.fillPath(buttonPath, backgroundColor);
  painter.drawPath(buttonPath);

  painter.setPen(QPen(fontColor));
  font.setWeight(QFont::Bold);
  painter.setFont(font);

  int textXCoord = (rect.width() - indexFontWidth) / 2;
  int textYCoord = rect.y() + fontMargin + fontHeight;
  painter.drawText(textXCoord, textYCoord, text());
 #endif
  // int btnCenterX = rect.width() / 2;
  int btnCenterY = rect.height() / 2;
 
  
  if(m_BadgeCount > 0)
  {
    m_BadgeDiameter = fontHeight + 3;
    QPainterPath badgePath;
    badgePath.addEllipse(rect.width() - m_BadgeDiameter - 3, btnCenterY - (m_BadgeDiameter / 2), m_BadgeDiameter, m_BadgeDiameter);
    QPen pen(badgeBgColor, 2);
    painter.setPen(pen);
    painter.fillPath(badgePath, badgeBgColor);
    painter.drawPath(badgePath);

    painter.setPen(QPen(Qt::white));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    QString number = QString::number(m_BadgeCount);
    int fw = fontMetrics.width(number);
    painter.drawText(rect.width() - 3 - (m_BadgeDiameter / 2) - (fw / 2), btnCenterY + (fontHeight / 3), number);
  }

}
