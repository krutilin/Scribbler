#include "svgeditor.h"

SvgEditor::SvgEditor(QWidget *parent) : QSvgWidget(parent)
{
    QPalette svgPalette = this->palette();
    svgPalette.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(svgPalette);
    inPoint = QPointF(-1.0, -1.0);
    outPoint = QPointF(-1.0, -1.0);
    limits = QRectF(-1.0,-1.0,-1.0,-1.0);
    scaleFactor = 1;
    minScaleFactor = 0.5;
    maxScaleFactor = 10;
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    drawInPoint = false;
    drawOutPoint = false;
    drawLimits = false;
    drawLetter = false;
}

void SvgEditor::load(const QString & file)
{
    QSvgWidget::load(file);
    drawLetter = true;
    inPoint = QPointF(-1.0, -1.0);
    outPoint = QPointF(-1.0, -1.0);
    limits = QRectF(-1.0,-1.0,-1.0,-1.0);
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    drawInPoint = false;
    drawOutPoint = false;
    drawLimits = false;
    showInPoint = false;
    showOutPoint = false;
    showLimits = false;
    update();
}
void SvgEditor::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    qreal newFactor = scaleFactor * factor;

    if (newFactor < maxScaleFactor && newFactor > minScaleFactor)
        scaleFactor = newFactor;

    update();
    event->accept();
}
void SvgEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (drawInPoint)
        setInPoint(event->pos());

    if (drawOutPoint)
        setOutPoint(event->pos());

    if (drawLimits)
        setLimitsTopLeft(event->pos());
}

void SvgEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (drawLimits)
        setLimitsBottomRight(event->pos());

    if (drawInPoint)
        setInPoint(event->pos());

    if (drawOutPoint)
        setOutPoint(event->pos());
}

void SvgEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (drawLimits)
    {
        setLimitsBottomRight(event->pos());

        if (limits.topLeft().x() > limits.bottomRight().x() ||
            limits.topLeft().y() > limits.bottomRight().y())
            limits = QRectF(limits.topRight(), limits.bottomLeft());
    }

    if (drawInPoint)
        setInPoint(event->pos());

    if (drawOutPoint)
        setOutPoint(event->pos());
}

void SvgEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

    renderer()->render(&painter, QRectF(QPointF(width() / 2 - renderer()->defaultSize().width() / 2 * scaleFactor,
                                                height() / 2 - renderer()->defaultSize().height() / 2 * scaleFactor),
                                        renderer()->defaultSize() *= scaleFactor));

    QPen pen = QPen(Qt::SolidPattern, pointWidth);

    if (showInPoint)
    {
        painter.save();
        pen.setColor(Qt::darkCyan);
        painter.setPen(pen);
        painter.drawPoint(fromStored(inPoint));
        painter.restore();
    }

    if (showOutPoint)
    {
        painter.save();
        pen.setColor(Qt::darkMagenta);
        painter.setPen(pen);
        painter.drawPoint(fromStored(outPoint));
        painter.restore();
    }

    if (showLimits)
    {
        painter.save();
        painter.setPen(Qt::darkYellow);
        painter.drawRect(QRectF(fromStored(limitsTopLeft), fromStored(limitsBottomRight)));
        painter.restore();
    }

    painter.end();

    event->accept();
}

void SvgEditor::setLetterData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits)
{
    inPoint = _inPoint;
    outPoint = _outPoint;
    limits = _limits;
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    showInPoint = true;
    showOutPoint = true;
    showLimits = true;
    update();
}

void SvgEditor::disableDrawing(bool disable)
{
    if (disable)
    {
        drawInPoint = false;
        drawOutPoint = false;
        drawLimits = false;
    }
    update();
}

void SvgEditor::enableInPointDrawing(bool draw)
{
    disableDrawing(true);
    drawInPoint = draw;
}

void SvgEditor::enableOutPointDrawing(bool draw)
{
    disableDrawing(true);
    drawOutPoint = draw;
}

void SvgEditor::enableLimitsDrawing(bool draw)
{
    disableDrawing(true);
    drawLimits = draw;
}

void SvgEditor::setInPoint(const QPointF &point)
{
    inPoint = toStored(point);
    showInPoint = true;
    update();
}

void SvgEditor::setOutPoint(const QPointF &point)
{
    outPoint = toStored(point);
    showOutPoint = true;
    update();
}

void SvgEditor::setLimitsTopLeft(const QPointF &point)
{
    limitsTopLeft = toStored(point);
}

void SvgEditor::setLimitsBottomRight(const QPointF &point)
{
    limitsBottomRight = point;

    if (limitsBottomRight.x() > this->width())
        limitsBottomRight.rx() = this->width() - 1;

    if (limitsBottomRight.y() > this->height())
        limitsBottomRight.ry() = this->height() - 1;

    limitsBottomRight = toStored(limitsBottomRight);

    limits = QRectF(limitsTopLeft, limitsBottomRight);
    showLimits = true;
    update();
}

QPointF SvgEditor::toStored(const QPointF &point)
{
    QPointF result;
    QSize currentLetterSize = renderer()->defaultSize() *= scaleFactor;
    QPointF letterBegin(width() / 2 - renderer()->defaultSize().width() / 2 * scaleFactor,
                        height() / 2 - renderer()->defaultSize().height() / 2 * scaleFactor);
    result.rx() = (point.x() - letterBegin.x()) / static_cast<qreal>(currentLetterSize.width() - 1);
    result.ry() = (point.y() - letterBegin.y()) / static_cast<qreal>(currentLetterSize.height() - 1);
    return result;
}

QPointF SvgEditor::fromStored(const QPointF &point)
{
    QPointF result;
    QSize currentLetterSize = renderer()->defaultSize() *= scaleFactor;
    QPointF letterBegin(width() / 2 - renderer()->defaultSize().width() / 2 * scaleFactor,
                        height() / 2 - renderer()->defaultSize().height() / 2 * scaleFactor);
    result.rx() = point.x() * static_cast<qreal>(currentLetterSize.width() - 1);
    result.ry() = point.y() * static_cast<qreal>(currentLetterSize.height() - 1);
    result += letterBegin;
    return result;
}