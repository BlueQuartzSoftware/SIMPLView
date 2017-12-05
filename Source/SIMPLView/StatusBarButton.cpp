#include "StatusBarButton.h"

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOption>

#include "SVWidgetsLib/QtSupport/QtSStyles.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatusBarButton::StatusBarButton(QWidget* parent)
: QPushButton(parent)
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
void StatusBarButton::paintEvent(QPaintEvent*)
{
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

  QRect rect = this->rect();

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
